all: build/dungeon

build/dungeon: src/*.c include/*.h buildDir
	gcc -std=c11 src/*.c -o build/dungeon -I include -Wall -Werror -g

test: tests/*.c src/*.c include/*.h buildDir
	gcc -std=c11 tests/*.c src/dungeon.c src/util.c -o build/tests -I include -Wall -Werror -g
	build/tests

clean:
	rm -rf build *.tar.gz

buildDir:
	@mkdir -p build
