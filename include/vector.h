#ifndef UTIL_HEADER_DEFINED
#define UTIL_HEADER_DEFINED

#include <stdlib.h>
#include <macros.h>

#define VECTOR_DEFAULT_LENGTH 5

// a basic vector
typedef struct {
	int allocLength;
	void *begin;
	int length;
	size_t elementSize;
} vector_t;

// initialize a vector
void vector_init(vector_t *vector, size_t, int allocLength);

// add to the end of the vector
void vector_add(vector_t *vector, void *value);

// get a pointer from a vector
#define vector_get(vector, type, index) \
	(*vector_get_ptr(vector, type, index))

// get a value in a vector
#define vector_get_ptr(vector, type, index) \
	(((type*) (vector)->begin) + index)

// remove the last value from a vector
void vector_remove_last(vector_t *vector);

// release the memory for a vector
void vector_destroy(vector_t *vector);

#define vector_for(type, var, vector) \
	for(type *var = (vector)->begin, *__end_ ## var = var + (vector)->length; var != __end_ ## var; ++var)

#endif
