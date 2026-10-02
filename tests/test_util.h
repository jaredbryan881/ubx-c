#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int test_failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			(void)fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
			test_failures++; \
		} \
	} while (0)

#define CHECK_EQ(actual, expected) \
	do { \
		const long long a_ = (long long)(actual); \
		const long long e_ = (long long)(expected); \
		if (a_ != e_) { \
			(void)fprintf(stderr, "%s:%d: %s == %lld, expected %lld\n", __FILE__, __LINE__, #actual, a_, e_); \
			test_failures++; \
		} \
	} while (0)

#define CHECK_BYTES(actual, expected, length) CHECK(memcmp((actual), (expected), (length)) == 0)

#define TEST_RESULT() (test_failures == 0 ? 0 : 1)

#endif
