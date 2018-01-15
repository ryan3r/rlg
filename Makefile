all: build/8queens


build/8queens: src/8queens.c
	gcc src/8queens.c -o build/8queens

test: build/8queens
	tests/queens

clean:
	rm -f *.tar.gz
	rm -f build/*
