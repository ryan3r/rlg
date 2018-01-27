# Based on Dr. Sheaffer's solution

GCC_FLAGS = -std=gnu11 -I include -Wall -Werror -ggdb

# objects for the program
SRC_OBJECTS = build/dungeon.o build/util.o build/heap.o
# objects for the tests
TEST_OBJECTS = build/dungeon_test.o

# build the program
build/dungeon: src/main.c $(SRC_OBJECTS)
	@echo Building $@
	@gcc $^ -o $@ $(GCC_FLAGS)

# run the tests
test: build/tests
	@build/tests

# build the tests
build/tests: $(TEST_OBJECTS) $(SRC_OBJECTS) tests/main.c
	@echo Building $@
	@gcc $^ -o build/tests $(GCC_FLAGS)

# include dependency files
-include $(SRC_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

# build individual files
build/%.o: tests/%.c
	@echo Building $@
	@gcc -c $< -o $@ $(GCC_FLAGS) -MMD -MF build/$*.d

build/%.o: src/%.c
	@echo Building $@
	@gcc -c $< -o $@ $(GCC_FLAGS) -MMD -MF build/$*.d

# clean up build artifacts
clean:
	@echo Removing all build files
	@rm -rf build/*

# build everything
all: build/tests build/dungeon

# build everything from scratch
rebuild: clean all
