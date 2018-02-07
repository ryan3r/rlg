# Based on Jeremy's solution

GCC_FLAGS = -std=gnu11 -I include -Wall -ggdb

# objects for the program
SRC_OBJECTS = build/dungeon.o build/vector.o build/heap.o build/arguments.o

# build the program
build/dungeon: build src/main.c $(SRC_OBJECTS)
	@echo Building $@
	@gcc src/main.c $(SRC_OBJECTS) -o $@ $(GCC_FLAGS)

# include dependency files
-include $(SRC_OBJECTS:.o=.d)

# build individual files
build/%.o: src/%.c
	@echo Building $@
	@gcc -c $< -o $@ $(GCC_FLAGS) -MMD -MF build/$*.d

# run memory tests
mem: build/dungeon
	valgrind --tool=memcheck --leak-check=yes build/dungeon

mem-load: build/dungeon
	valgrind --tool=memcheck --leak-check=yes build/dungeon --load

mem-save: build/dungeon
	valgrind --tool=memcheck --leak-check=yes build/dungeon --save

# clean up build artifacts
clean:
	@echo Removing all build files
	@rm -rf build

# build everything from scratch
rebuild: clean build/dungeon

# create the build directory
build:
	@mkdir -p build

# run tests
test: build/dungeon
	@build/dungeon --save --player="(1,1)" > expected_outputs/expected
	@build/dungeon --load --player="(1,1)" > expected_outputs/out
	@diff expected_outputs/out expected_outputs/expected

	@build/dungeon --player="(10,15)" --load="$(shell pwd)/test_dungeon_files/1521618087.rlg327" > expected_outputs/out
	@diff expected_outputs/out expected_outputs/1521618087.rlg327

	@echo "All tests passed"