
#ifndef _RB_H
#define _RB_H 1

#include <proton/types.h>
#include <time.h>

#include <pthread.h>

typedef struct {
    pn_rwbytes_t *ring_buffer;

    int count;
    int buf_size;

    volatile int head;
    volatile int tail;

    pthread_mutex_t rb_mutex;
    pthread_cond_t rb_ready;

    // stats
    //
    // Buffer full
    volatile long overruns;
    // Number of messages procesed
    volatile long processed;
    volatile long queue_block;

    struct timespec total_active, total_wait;
    struct timespec total_t1, total_t2;

} rb_rwbytes_t;

extern rb_rwbytes_t *rb_alloc(int count, int buf_size);

extern pn_rwbytes_t *rb_get_head(rb_rwbytes_t *rb);

extern pn_rwbytes_t *rb_get_tail(rb_rwbytes_t *rb);

extern pn_rwbytes_t *rb_put(rb_rwbytes_t *rb);

extern pn_rwbytes_t *rb_get(rb_rwbytes_t *rb);

extern void rb_free(rb_rwbytes_t *rb);

extern int rb_free_size(rb_rwbytes_t *rb);

extern int rb_inuse_size(rb_rwbytes_t *rb);

extern int rb_size(rb_rwbytes_t *rb);

extern long rb_get_queue_block(rb_rwbytes_t *rb);

#endif