/*
 * Copyright 2024 Red Hat, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "bridge.h"
#include "minunit.h"
#include "utils.h"
#include <stdio.h>

/***************** test suite *****************/

static char *test_match_amqp_url_hostname() {
    char *matches[10];
    memset(matches, 0, sizeof(matches));
    match_regex(AMQP_URL_REGEX, matches, 10,
                "amqp://scooby:doo@some.k8s.svc:5666/foo/bar");

    mu_assert("Failed to parse full amqp_url in form of hostname: user",
              !strcmp(matches[3], "scooby"));
    mu_assert("Failed to parse full amqp_url in form of hostname: password",
              !strcmp(matches[5], "doo"));
    mu_assert("Failed to parse full amqp_url in form of hostname: host",
              !strcmp(matches[6], "some.k8s.svc"));
    mu_assert("Failed to parse full amqp_url in form of hostname: port",
              !strcmp(matches[8], "5666"));
    mu_assert("Failed to parse full amqp_url in form of hostname: address",
              !strcmp(matches[9], "/foo/bar"));

    char *matches2[10];
    memset(matches2, 0, sizeof(matches2));
    match_regex(AMQP_URL_REGEX, matches2, 10, "amqps://other.k8s.svc:5666/baz");

    mu_assert(
        "Failed to parse amqp_url w/o user info in form of hostname: user",
        matches2[3] == NULL);
    mu_assert(
        "Failed to parse amqp_url w/o user info in form of hostname: password",
        matches2[5] == NULL);
    mu_assert(
        "Failed to parse amqp_url w/o user info in form of hostname: host",
        !strcmp(matches2[6], "other.k8s.svc"));
    mu_assert(
        "Failed to parse amqp_url w/o user info in form of hostname: port",
        !strcmp(matches2[8], "5666"));
    mu_assert(
        "Failed to parse amqp_url w/o user info in form of hostname: address",
        !strcmp(matches2[9], "/baz"));

    return 0;
}

static char *test_match_amqp_url_ipv4() {
    char *matches[10];
    memset(matches, 0, sizeof(matches));
    match_regex(AMQP_URL_REGEX, matches, 10,
                "amqp://scooby:doo@127.0.0.1:5666/foo/bar");

    mu_assert("Failed to parse amqp_url in form of IPv4: user invalid: %s",
              !strcmp(matches[3], "scooby"));
    mu_assert("Failed to parse amqp_url in form of IPv4: password",
              !strcmp(matches[5], "doo"));
    mu_assert("Failed to parse amqp_url in form of IPv4:i host",
              !strcmp(matches[6], "127.0.0.1"));
    mu_assert("Failed to parse amqp_url in form of IPv4: port",
              !strcmp(matches[8], "5666"));
    mu_assert("Failed to parse amqp_url in form of IPv4: address",
              !strcmp(matches[9], "/foo/bar"));

    char *matches2[10];
    memset(matches2, 0, sizeof(matches2));
    match_regex(AMQP_URL_REGEX, matches2, 10, "amqps://127.0.0.1:5666/baz");

    mu_assert("Failed to parse amqp_url w/o user info in form of IPv4: user",
              matches2[3] == NULL);
    mu_assert(
        "Failed to parse amqp_url w/o user info in form of IPv4: password",
        matches2[5] == NULL);
    mu_assert("Failed to parse amqp_url w/o user info in form of IPv4: host",
              !strcmp(matches2[6], "127.0.0.1"));
    mu_assert("Failed to parse amqp_url w/o user info in form of IPv4: port",
              !strcmp(matches2[8], "5666"));
    mu_assert("Failed to parse amqp_url w/o user info in form of IPv4: address",
              !strcmp(matches2[9], "/baz"));
    return 0;
}

static char *test_match_amqp_url_ipv6() {
    char *matches[10];
    memset(matches, 0, sizeof(matches));
    match_regex(AMQP_URL_REGEX, matches, 10,
                "amqp://scooby:doo@[fe80::abcd:fcff:fe07:9999]:5666/foo/bar");

    mu_assert("Failed to parse amqp_url in form of IPv6: user",
              !strcmp(matches[3], "scooby"));
    mu_assert("Failed to parse amqp_url in form of IPv6: password",
              !strcmp(matches[5], "doo"));
    mu_assert("Failed to parse amqp_url in form of IPv6: host",
              !strcmp(matches[6], "[fe80::abcd:fcff:fe07:9999]"));
    mu_assert("Failed to parse amqp_url in form of IPv6: port",
              !strcmp(matches[8], "5666"));
    mu_assert("Failed to parse amqp_url in form of IPv6: address",
              !strcmp(matches[9], "/foo/bar"));

    char *matches2[10];
    memset(matches2, 0, sizeof(matches2));
    match_regex(AMQP_URL_REGEX, matches2, 10,
                "amqps://[fe80::abcd:fcff:fe07:9999]:5666/bas");

    mu_assert("Failed to parse amqp_url w/o user info in form of IPv6: user",
              matches2[3] == NULL);
    mu_assert(
        "Failed to parse amqp_url w/o user info in form of IPv6: password",
        matches2[5] == NULL);
    mu_assert("Failed to parse amqp_url w/o user info in form of IPv6: host",
              !strcmp(matches2[6], "[fe80::abcd:fcff:fe07:9999]"));
    mu_assert("Failed to parse amqp_url w/o user info in form of IPv6: port",
              !strcmp(matches2[8], "5666"));
    mu_assert("Failed to parse amqp_url w/o user info in form of IPv6: address",
              !strcmp(matches2[9], "/bas"));
    return 0;
}

static char *test_match_amqp_url_fail() {
    char *matches[10];
    memset(matches, 0, sizeof(matches));
    match_regex(AMQP_URL_REGEX, matches, 10,
                "amqp://scooby:doo@[XXX.666.000.123/64]:5666/foo/bar");

    mu_assert("Failed to fail parsing invalid amqp_url",
              matches[6] == NULL && matches[9] == NULL);
    return 0;
}

/******************* runner *******************/

int tests_run = 0;

static char *all_tests() {
    mu_run_test(test_match_amqp_url_hostname);
    mu_run_test(test_match_amqp_url_ipv4);
    mu_run_test(test_match_amqp_url_ipv6);
    mu_run_test(test_match_amqp_url_fail);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
