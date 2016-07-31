#ifndef SOCKETVECTOR_H
#define SOCKETVECTOR_H

typedef struct{

	unsigned int size;
	unsigned int capacity;
	unsigned int *buffer;

} socketVector;

void svInit(socketVector *vec, unsigned int startCapacity);
unsigned int svGet(socketVector *vec, unsigned int index);
void svResize(socketVector *vec, unsigned int newCapacity);
void svPush(socketVector *vec, const int element);
void svErase(socketVector *vec, unsigned int index);
void svClear(socketVector *vec);

#endif
