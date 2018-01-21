all: build/dungeon

build/dungeon: src/*.c include/*.h buildDir
	gcc -std=c11 src/*.c -o build/dungeon -I include -Wall -Werror -g

test: build/dungeon


clean:
	rm -f *.tar.gz
	rm -rf build

buildDir:
	@mkdir -p build
