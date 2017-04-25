#ifndef CVECTOR_H
#define CVECTOR_H

#include <stdlib.h>

enum type {
	VOID_T,
	CHAR_T,
	INT_T,
	LONG_T,
	FLOAT_T,
	DOUBLE_T,
	LONG_DOUBLE_T
};

typedef struct cVector {
	size_t size;
	size_t capacity;
	void **buffer;
} cVector;

void cvInit(cVector *vec, unsigned int startCapacity);
void cvResize(cVector *vec, size_t capacity);
void cvPush(cVector *vec, void *data, enum type dataType, unsigned int length);
void cvPop(cVector *vec);
void cvInsert(cVector *vec, unsigned int pos, void *data, enum type dataType, unsigned int length);
void cvErase(cVector *vec, unsigned int pos);
void *cvGet(cVector *vec, unsigned int pos);
void cvSet(cVector *vec, unsigned int pos, void *data, enum type dataType, unsigned int length);
size_t cvSize(cVector *vec);
void cvClear(cVector *vec);

#endif
