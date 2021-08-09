#ifndef _UTILS_H
#define _UTILS_H 1

#include <time.h>

void time_diff(struct timespec t1, struct timespec t2, struct timespec *diff);
char *time_snprintf(char *buf, size_t n, struct timespec t1);

#endif
