#ifndef UTIL_HEADER_DEFINED
#define UTIL_HEADER_DEFINED

#include <stdlib.h>

// the typical for each loop
#define FOR(var, end) \
	for(unsigned int var = 0; var < end; ++var)

// a basic vector
typedef struct {
	int allocLength;
	void *begin;
	int length;
	size_t elementSize;
} vector_t;

// initialize a vector
void vector_init(vector_t *vector, size_t);

// add to the end of the vector
void vector_add(vector_t *vector, void *value);

// get a value in a vector
#define vector_get(vector, type, index) \
	*(type*) ((vector)->begin + index * (vector)->elementSize)

// remove the last value from a vector
void vector_remove_last(vector_t *vector);

// release the memory for a vector
void vector_destroy(vector_t *vector);

// get the size of the vector
unsigned int vector_size(vector_t *vector);

// get the end pointer for the vector
void* vector_end(vector_t *vector);

#define vector_for(type, var, vector) \
	for(type *var = (vector)->begin, *__end_ ## var = vector_end(vector); var < __end_ ## var; ++var)

#endif
