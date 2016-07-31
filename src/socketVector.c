#include "socketVector.h"
#include <stdlib.h>
//#include <stdio.h>

void svInit(socketVector *vec, unsigned int startCapacity){
	vec->size = 0;
	vec->capacity = startCapacity;
	vec->buffer = malloc(4 * startCapacity);
}

unsigned int svGet(socketVector *vec, unsigned int index){
	if(index < vec->size){
		return vec->buffer[index];
	}else{
		//printf("Vector error: element %i does not exist.\n", index);
		return vec->buffer[vec->size - 1];
	}
}

void svResize(socketVector *vec, unsigned int newCapacity){
	/*if(vec->size > newCapacity){
		printf("Vector warning: the new capacity of the vector is less than the number of elements it contains.\n");
	}*/
	if(newCapacity != vec->capacity){
		unsigned int *tempBuffer = realloc(vec->buffer, 4 * newCapacity);
		if(tempBuffer != NULL){
			vec->buffer = tempBuffer;
		}/*else{
			printf("Vector error: realloc() returned NULL.\n");
		}*/
	}
}

void svPush(socketVector *vec, const int element){
	if(vec->capacity == 0){
		vec->capacity = 1;
		vec->buffer = malloc(4);
	}else if(vec->size == vec->capacity){
		vec->capacity *= 2;
		svResize(vec, vec->capacity);
	}
	vec->buffer[vec->size++] = element;
}

void svPop(socketVector *vec){
	vec->buffer[vec->size] = 0;
	vec->size--;
}

void svErase(socketVector *vec, unsigned int index){
	int nextElement;
	unsigned int d;
	for(d = index; d < vec->size; d++){
		nextElement = 0;
		if(d + 1 < vec->size){
			nextElement = vec->buffer[d + 1];
		}
		vec->buffer[d] = nextElement;
	}
	vec->size--;
}

void svClear(socketVector *vec){
	vec->size = 0;
	vec->capacity = 0;
	free(vec->buffer);
}
