# Based on Jeremy's solution

GCC_FLAGS = -std=gnu11 -I include -Wall -Werror -ggdb $(EXTRA_FLAGS)

# objects for the program
SRC_OBJECTS = build/dungeon.o build/vector.o build/heap.o build/arguments.o

# build the program
build/dungeon: src/main.c $(SRC_OBJECTS)
	@echo Building $@
	@gcc $^ -o $@ $(GCC_FLAGS)

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
	@rm -rf build/*

# build everything from scratch
rebuild: clean build/dungeon
