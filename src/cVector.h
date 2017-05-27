#ifndef CVECTOR_H
#define CVECTOR_H

#include <stdlib.h>

typedef struct cVector {
	size_t size;
	size_t capacity;
	void **buffer;
} cVector;

void cvInit(cVector *vec, unsigned int startCapacity);
void cvResize(cVector *vec, size_t capacity);
void cvPush(cVector *vec, void *data, unsigned int bytes);
void cvPop(cVector *vec);
void cvInsert(cVector *vec, unsigned int pos, void *data, unsigned int bytes);
void cvErase(cVector *vec, unsigned int pos);
void *cvGet(cVector *vec, unsigned int pos);
void cvSet(cVector *vec, unsigned int pos, void *data, unsigned int bytes);
size_t cvSize(cVector *vec);
void cvClear(cVector *vec);

#endif
