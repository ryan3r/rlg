#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// make default to 8 queens
#ifndef SIZE
#define SIZE 8
#endif

// track how many queens we have
int numQueens = 0;

// check if there is a queen in this row
bool hasQueen(int queens[][2], int i) {
	int k;

	for(k = 0; k < numQueens; ++k) {
		if(queens[k][0] == i) {
			return true;
		}
	}

	return false;
}

// check if a queen can be placed here
bool isOpen(int queens[][2], int i, int j) {
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
bool updateQueen(int queens[][2], int i, int j, bool place) {
	int k;
	int variant = place ? 1 : -1;

	// we can place a queen
	if(isOpen(queens, i, j) || !place) {
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
void printPlacements(int queens[][2]) {
	int i;

	// convert queens to string points
	for(i = 0; i < numQueens; ++i) {
		printf("%c%d", 'a' + queens[i][0], queens[i][1]);
	}

	printf("\n");
}

// find a place for a queen
void placeQueen(int queens[][2]) {
	int i, j = 0;
	bool queenPlaced = false;

	// check every spot in then board for an open space
	for(i = 0; i < SIZE; ++i) {
		// already a queen here
		if(hasQueen(queens, i)) continue;

		for(; j < SIZE; ++j) {
			// try to place a queen in this spot
			bool placed = updateQueen(queens, i, j, true);

			if(placed) {
				// we have reached the number of queens
				if(numQueens == SIZE) {
					printPlacements(queens);
				}
				// start another queen
				else {
					placeQueen(queens);
				}

				// remove this queen
				updateQueen(queens, i, j, false);
			}
		}
	}
}

// initialize the queens and start
int main(int argc, char **argv) {
	int queens[SIZE][2];

	placeQueen(queens);
}
