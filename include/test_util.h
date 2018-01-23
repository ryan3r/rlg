#define ASSERT(cond, msg) \
	if(!cond) { \
		fprintf(stderr, "[\x1b[31mFAIL\x1b[0m]: " msg "\n"); \
		test_failed = true; \
	} \
	else { \
		printf("[\x1b[32m OK \x1b[0m]: " msg "\n"); \
	}

#define XASSERT(cond, msg) \
	printf("[\x1b[36m----\x1b[0m]: " msg "\n");
