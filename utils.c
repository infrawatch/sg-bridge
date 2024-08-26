#include "utils.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

void time_diff(struct timespec t1, struct timespec t2, struct timespec *diff) {
    if (t2.tv_nsec < t1.tv_nsec) {
        /* If nanoseconds in t1 are larger than nanoseconds in t2, it
           means that something like the following happened:
           t1.tv_sec = 1000    t1.tv_nsec = 100000
           t2.tv_sec = 1001    t2.tv_nsec = 10
           In this case, less than a second has passed but subtracting
           the tv_sec parts will indicate that 1 second has passed. To
           fix this problem, we subtract 1 second from the elapsed
           tv_sec and add one second to the elapsed tv_nsec. See
           below:
        */
        diff->tv_sec += t2.tv_sec - t1.tv_sec - 1;
        diff->tv_nsec += t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff->tv_sec += t2.tv_sec - t1.tv_sec;
        diff->tv_nsec += t2.tv_nsec - t1.tv_nsec;
    }
}

char *time_snprintf(char *buf, size_t n, struct timespec t1) {
    double pct = (t1.tv_sec * 1000000000.0) + t1.tv_nsec / 1000000000.0;

    snprintf(buf, n, "%f", pct);

    return buf;
}

int match_regex(char *regmatch, char *matches[], int n_matches,
                const char *to_match) {
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];
    regex_t regex;

    if (regcomp(&regex, regmatch, REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex: %s\n", regmatch);

        return -1;
    }

    int nomatch = regexec(&regex, to_match, n_matches, m, 0);
    if (nomatch == REG_NOMATCH) {
        return 0;
    }

    int match_count = 0;
    for (int i = 0; i < n_matches; i++) {
        if (m[i].rm_so == -1) {
            continue;
        }
        match_count++;

        int match_len = m[i].rm_eo - m[i].rm_so;

        matches[i] = malloc(match_len + 1); // make room for '\0'

        int k = 0;
        for (int j = m[i].rm_so; j < m[i].rm_eo; j++) {
            matches[i][k++] = to_match[j];
        }
        matches[i][k] = '\0';
    }

    return match_count;
}
