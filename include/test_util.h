#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// test an assertion and print the result
#define ASSERT(cond, msg) \
	if(!cond) { \
		fprintf(stderr, "[\x1b[31mFAIL\x1b[0m]: " msg "\n"); \
		++__tests_failed; \
	} \
	else { \
		if(verbose_mode) printf("[\x1b[32m OK \x1b[0m]: " msg "\n"); \
		++__tests_passed; \
	}

// a disabled assertion
#define XASSERT(cond, msg) \
	if(verbose_mode) printf("[\x1b[36m----\x1b[0m]: " msg "\n"); \
	++__tests_disabled;

// track whether a test passed, failed, or was disabled
extern uint32_t __tests_failed;
extern uint32_t __tests_passed;
extern uint32_t __tests_disabled;

extern bool verbose_mode;

#endif
