#include <stdlib.h>
#include <vector.h>
#include <string.h>

// initialize a vector
void vector_init(vector_t *vector, size_t size, int allocLength) {
	vector->length = 0;
	vector->allocLength = allocLength;
	vector->elementSize = size;
	vector->begin = malloc(vector->allocLength * size);
}

// add to the end of the vector
void vector_add(vector_t *vector, void *value) {
	++vector->length;

	// resize the vector
	if(vector->length > vector->allocLength) {
		vector->allocLength *= 2;
		vector->begin = realloc(vector->begin,
			vector->allocLength * vector->elementSize);
	}

	memcpy(vector->begin + (vector->length - 1) * vector->elementSize, value, vector->elementSize);
}

// remove the last value from a vector
void vector_remove_last(vector_t *vector) {
	--vector->length;
}

// release the memory for a vector
void vector_destroy(vector_t *vector) {
	free(vector->begin);
}
