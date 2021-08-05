#define _GNU_SOURCE
#include <features.h>

#include <assert.h>
#include <proton/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rb.h"
#include "utils.h"

rb_rwbytes_t *rb_alloc(int count, int buf_size, bool block_producer) {
    rb_rwbytes_t *rb = malloc(sizeof(rb_rwbytes_t));

    rb->count = count;
    rb->buf_size = buf_size;
    rb->block_producer = block_producer;

    if ((rb->ring_buffer = malloc(count * sizeof(pn_rwbytes_t))) == NULL) {
        free(rb);

        return NULL;
    }

    for (int i = 0; i < count; i++) {
        if ((rb->ring_buffer[i].start = malloc(buf_size)) == NULL) {
            rb_free(rb);

            return NULL;
        }
        rb->ring_buffer[i].size = 0;
    }
    rb->head = 0;
    rb->tail = count - 1;

    rb->overruns = 0;
    rb->processed = 0;
    rb->queue_block = 0;

    rb->total_active.tv_sec = 0;
    rb->total_active.tv_nsec = 0;

    rb->total_wait.tv_sec = 0;
    rb->total_wait.tv_nsec = 0;

    rb->total_t1.tv_sec = 0;
    rb->total_t1.tv_nsec = 0;

    rb->total_t2.tv_sec = 0;
    rb->total_t2.tv_nsec = 0;

    pthread_cond_init(&rb->rb_ready, NULL);
    pthread_mutex_init(&rb->rb_mutex, NULL);

    if (rb->block_producer) {
        pthread_cond_init(&rb->rb_free, NULL);
    }

    return rb;
}

void rb_free(rb_rwbytes_t *rb) {
    if (rb == NULL) {
        return;
    }
    for (int i = 0; i < rb->count; i++) {
        free(rb->ring_buffer[i].start);
    }
    free(rb->ring_buffer);
    free(rb);
}

pn_rwbytes_t *rb_get_head(rb_rwbytes_t *rb) {
    if (rb == NULL) {
        return NULL;
    }
    return &rb->ring_buffer[rb->head];
}

pn_rwbytes_t *rb_get_tail(rb_rwbytes_t *rb) {
    if (rb == NULL) {
        return NULL;
    }
    return &rb->ring_buffer[rb->tail];
}

// Place the already allocated buffer entry in the
// queue. The producer does not block unless the
// --amqp_block flag is specified, as it needs
// to continually process the incoming AMQP messages.
// Just need to wake up the consumer if it is waiting
// for messages.
pn_rwbytes_t *rb_put(rb_rwbytes_t *rb) {
    if (rb == NULL) {
        return NULL;
    }
    pn_rwbytes_t *next_buffer = NULL;

    int next = (rb->head + 1) % rb->count;
    if (rb->block_producer && next == rb->tail) {
        // Wait until consumer consumes a message,
        // which frees a space in rb and that makes
        // (next != rb-> tail) == true
        pthread_mutex_lock(&rb->rb_mutex);
        pthread_cond_wait(&rb->rb_free, &rb->rb_mutex);
        pthread_mutex_unlock(&rb->rb_mutex);
    }
    if (next != rb->tail) {
        rb->head = next;
        next_buffer = &rb->ring_buffer[rb->head];
        pthread_mutex_lock(&rb->rb_mutex);
        pthread_cond_broadcast(&rb->rb_ready);
        pthread_mutex_unlock(&rb->rb_mutex);
    } else {
        rb->overruns++;
        rb->ring_buffer[rb->head].size = 0;
    }

    return next_buffer; // May be NULL
}

pn_rwbytes_t *rb_get(rb_rwbytes_t *rb) {
    if (rb == NULL) {
        return NULL;
    }

    int next;

    next = (rb->tail + 1) % rb->count;
    while (next == rb->head) {
        pthread_mutex_lock(&rb->rb_mutex);
        pthread_cond_wait(&rb->rb_ready, &rb->rb_mutex);
        pthread_mutex_unlock(&rb->rb_mutex);

        next = (rb->tail + 1) % rb->count;
        rb->queue_block++;
    }
    // set data size to zero
    rb->ring_buffer[rb->tail].size = 0;

    rb->tail = next;

    if (rb->block_producer) {
        pthread_mutex_lock(&rb->rb_mutex);
        pthread_cond_broadcast(&rb->rb_free);
        pthread_mutex_unlock(&rb->rb_mutex);
    }

    rb->processed++;

    return &rb->ring_buffer[rb->tail];
}

int rb_inuse_size(rb_rwbytes_t *rb) { return rb->count - rb_free_size(rb); }

int rb_free_size(rb_rwbytes_t *rb) {
    assert(rb->head != rb->tail);

    int head = rb->head;
    int tail = rb->tail;

    return head > tail ? rb->count - (head - tail) : tail - head;
}

int rb_size(rb_rwbytes_t *rb) { return rb->count; }

long rb_get_overruns(rb_rwbytes_t *rb) { return rb->overruns; }

long rb_get_processed(rb_rwbytes_t *rb) { return rb->processed; }

long rb_get_queue_block(rb_rwbytes_t *rb) { return rb->queue_block; }
