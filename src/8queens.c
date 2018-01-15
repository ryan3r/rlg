#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// track how many queens we have
int numQueens = 0;
// the number of queens we want to get
int size;
// the list of queens
int **queens;

// check if there is a queen in this row
bool hasQueen(int i) {
	int k;

	for(k = 0; k < numQueens; ++k) {
		if(queens[k][0] == i) {
			return true;
		}
	}

	return false;
}

// check if a queen can be placed here
bool isOpen(int i, int j) {
	int k;

	for(k = 0; k < numQueens; ++k) {
		// queen in same row or column
		if(queens[k][0] == i || queens[k][1] == j) return false;

		// check diagonals
		if(fabs(queens[k][0] - i) == fabs(queens[k][1] - j)) return false;
	}

	return true;
}

// place/remove a queen in a spot
bool updateQueen(int i, int j, bool place) {
	int k;
	int variant = place ? 1 : -1;

	// we can place a queen
	if(isOpen(i, j) || !place) {
		numQueens += variant;

		if(place) {
			queens[numQueens - 1][0] = i;
			queens[numQueens - 1][1] = j;
		}

		return true;
	}

	return false;
}

// print the queen locations
void printPlacements() {
	int i;

	// convert queens to string points
	for(i = 0; i < numQueens; ++i) {
		printf("%c%d", 'a' + queens[i][0], queens[i][1]);
	}

	printf("\n");
}

// find a place for a queen
void placeQueen() {
	int i, j = 0;
	bool queenPlaced = false;

	// check every spot in then board for an open space
	for(i = 0; i < size; ++i) {
		// already a queen here
		if(hasQueen(i)) continue;

		for(; j < size; ++j) {
			// try to place a queen in this spot
			bool placed = updateQueen(i, j, true);

			if(placed) {
				// we have reached the number of queens
				if(numQueens == size) {
					printPlacements();
				}
				// start another queen
				else {
					placeQueen();
				}

				// remove this queen
				updateQueen(i, j, false);
			}
		}
	}
}

// initialize the queens and start
int main(int argc, char **argv) {
	int i;

	// use the number of queens we are given
	if(argc > 1) {
		size = atoi(argv[1]);
	}
	// default to 8 queens
	else {
		size = 8;
	}

	// allocate the queens array
	queens = malloc(size * sizeof(int*));

	for(i = 0; i < size; ++i) {
		queens[i] = malloc(2 * sizeof(int));
	}

	placeQueen();
}
