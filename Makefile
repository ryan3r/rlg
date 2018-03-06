# Based on Jeremy's solution

GCC_FLAGS = -std=gnu11 -I include -Wall -ggdb
GPP_FLAGS = -I include -Wall -ggdb

# get all the objects based on the c files
SRC_OBJECTS = $(foreach file, $(shell echo src/*), \
	 build/$(shell basename $(shell basename $(file) .cpp) .c).o)

# build the program
build/rlg: build $(SRC_OBJECTS)
	@echo Linking
	@gcc $(SRC_OBJECTS) -o $@ $(GCC_FLAGS) -lncurses

# include dependency files
-include build/*.d

# build individual files
build/%.o: src/%.c
	@echo Building $(shell basename $@ .o)
	@gcc -c $< -o $@ $(GCC_FLAGS) -MMD -MF build/$*.d

build/%.o: src/%.cpp
	@echo Building $(shell basename $@ .o)
	@g++ -c $< -o $@ $(GPP_FLAGS) -MMD -MF build/$*.d

# run memory tests
mem: build/rlg
	valgrind --tool=memcheck --leak-check=yes build/rlg

mem-load: build/rlg
	valgrind --tool=memcheck --leak-check=yes build/rlg --load

mem-save: build/rlg
	valgrind --tool=memcheck --leak-check=yes build/rlg --save

# clean up build artifacts
clean:
	@echo Removing all build files
	@rm -rf build

# create the build directory
build:
	@mkdir -p build

.PHONY: mem mem-save mem-load clean