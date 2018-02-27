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

# build a deb
build/rlg.deb: build/rlg
	@echo Packaging
	@mkdir -p /tmp/rlg/DEBIAN
	@mkdir -p /tmp/rlg/usr/local/bin
	@cp deb-manifest /tmp/rlg/DEBIAN/control
	@cp build/rlg /tmp/rlg/usr/local/bin
	@chmod -R 0775 /tmp/rlg
	@dpkg-deb --build /tmp/rlg > /dev/null
	@dpkg-sig --sign builder /tmp/rlg.deb
	@rm -r /tmp/rlg
	@mv /tmp/rlg.deb build

.PHONY: mem mem-save mem-load clean