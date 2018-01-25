GCC_FLAGS = -std=c11 -I include -Wall -Werror -g

# objects for the program
SRC_OBJECTS = build/dungeon.o build/util.o
# objects for the tests
TEST_OBJECTS = build/dungeon_test.o


# build the program
build/dungeon: src/main.c $(SRC_OBJECTS)
	@gcc $^ -o $@ $(GCC_FLAGS)

# build the tests
test: $(TEST_OBJECTS) $(SRC_OBJECTS) tests/main.c
	@gcc $^ -o build/tests $(GCC_FLAGS)
	@build/tests

# build individual files
build/%.o: tests/%.c
	@gcc -c $^ -o $@ $(GCC_FLAGS)

build/%.o: src/%.c
	@gcc -c $^ -o $@ $(GCC_FLAGS)

# clean up build artifacts
clean:
	@rm -rf build/*
