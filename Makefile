# Based on Jeremy's solution

GCC_FLAGS = -std=gnu11 -I include -Wall -ggdb

# objects for the program
SRC_OBJECTS = build/dungeon.o build/vector.o build/heap.o build/arguments.o

# build the program
build/rlg: build src/main.c $(SRC_OBJECTS)
	@echo Building $@
	@gcc src/main.c $(SRC_OBJECTS) -o $@ $(GCC_FLAGS)

# include dependency files
-include $(SRC_OBJECTS:.o=.d)

# build individual files
build/%.o: src/%.c
	@echo Building $@
	@gcc -c $< -o $@ $(GCC_FLAGS) -MMD -MF build/$*.d

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

# build everything from scratch
rebuild: clean build/rlg

# create the build directory
build:
	@mkdir -p build

# run tests
test: build/rlg
	@build/rlg --save --player="(1,1)" > expected_outputs/expected
	@build/rlg --load --player="(1,1)" > expected_outputs/out
	@diff expected_outputs/out expected_outputs/expected

	@rm -f expected_outputs/out

	@echo "test_dungeon_files/1521618087.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 5, 60" >> expected_outputs/out
	@build/rlg --player="(5,60)" --load="$(shell pwd)/test_dungeon_files/1521618087.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1522914515.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 6, 24" >> expected_outputs/out
	@build/rlg --player="(6,24)" --load="$(shell pwd)/test_dungeon_files/1522914515.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1523530501.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 6, 53" >> expected_outputs/out
	@build/rlg --player="(6,53)" --load="$(shell pwd)/test_dungeon_files/1523530501.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1524171099.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 5, 43" >> expected_outputs/out
	@build/rlg --player="(5,43)" --load="$(shell pwd)/test_dungeon_files/1524171099.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1524787644.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 9, 40" >> expected_outputs/out
	@build/rlg --player="(9,40)" --load="$(shell pwd)/test_dungeon_files/1524787644.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1525363666.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 4, 31" >> expected_outputs/out
	@build/rlg --player="(4,31)" --load="$(shell pwd)/test_dungeon_files/1525363666.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1525972216.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 9, 54" >> expected_outputs/out
	@build/rlg --player="(9,54)" --load="$(shell pwd)/test_dungeon_files/1525972216.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1526652288.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 8, 49" >> expected_outputs/out
	@build/rlg --player="(8,49)" --load="$(shell pwd)/test_dungeon_files/1526652288.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1527308739.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 9, 7" >> expected_outputs/out
	@build/rlg --player="(9,7)" --load="$(shell pwd)/test_dungeon_files/1527308739.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/1527997509.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 8, 18" >> expected_outputs/out
	@build/rlg --player="(8,18)" --load="$(shell pwd)/test_dungeon_files/1527997509.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/adventure.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 6, 55" >> expected_outputs/out
	@build/rlg --player="(6,55)" --load="$(shell pwd)/test_dungeon_files/adventure.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/hello.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 11, 7" >> expected_outputs/out
	@build/rlg --player="(11,7)" --load="$(shell pwd)/test_dungeon_files/hello.rlg327" >> expected_outputs/out
	@echo "test_dungeon_files/welldone.rlg327" >> expected_outputs/out
	@echo "PC is at (y, x): 5, 71" >> expected_outputs/out
	@build/rlg --player="(5,71)" --load="$(shell pwd)/test_dungeon_files/welldone.rlg327" >> expected_outputs/out

	@diff expected_outputs/out test_dungeon_files/distance_maps.txt

	@echo "All tests passed"

.PHONY: test rebuild mem mem-save mem-load clean