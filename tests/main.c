#include <test_util.h>
#include <string.h>

uint32_t __tests_failed = 0;
uint32_t __tests_passed = 0;
uint32_t __tests_disabled = 0;

bool verbose_mode = false;

// test headers
void dungeon_test();

int main(int argc, char **argv) {
	// check for the verbose flag
	if(argc > 1) {
		verbose_mode = !strcmp(argv[1], "-v") || !strcmp(argv[1], "--verbose");
	}

	// run tests
	dungeon_test();

	// if we reached here all tests passed
	printf("%d passed, %d failed, %d disabled\n", __tests_passed, __tests_failed, __tests_disabled);

	return __tests_failed > 0 ? 1 : 0;
}
