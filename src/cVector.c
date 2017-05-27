#include "cVector.h"
#include <string.h>

void cvInit(cVector *vec, unsigned int startCapacity){
	vec->size = 0;
	vec->capacity = startCapacity;
	vec->buffer = malloc(sizeof(void *) * startCapacity);
}

void cvResize(cVector *vec, size_t capacity){
	void **newData = realloc(vec->buffer, sizeof(void *) * capacity);
	if(newData != NULL){
		vec->buffer = newData;
		vec->capacity = capacity;
	}
}

void cvPush(cVector *vec, void *data, unsigned int bytes){
	if(vec->size == vec->capacity){
		cvResize(vec, vec->capacity * 2);
	}
	vec->buffer[vec->size] = malloc(bytes);
	memcpy(vec->buffer[vec->size], data, bytes);
	vec->size++;
}

void cvPop(cVector *vec){
	free(vec->buffer[--vec->size]);
}

void cvInsert(cVector *vec, unsigned int pos, void *data, unsigned int bytes){
	if(pos < vec->size){
		if(vec->size == vec->capacity){
			cvResize(vec, vec->capacity * 2);
		}
		unsigned int i;
		for(i = pos + 1; i < vec->size; i++){
			vec->buffer[i] = vec->buffer[i - 1];
		}
		free(vec->buffer[pos]);
		vec->buffer[pos] = malloc(bytes);
		memcpy(vec->buffer[pos], data, bytes);
		vec->size++;
	}else{
		cvPush(vec, data, bytes);
	}
}

void cvErase(cVector *vec, unsigned int pos){
	if(pos < vec->size){
		free(vec->buffer[pos]);
		unsigned int i;
		for(i = pos; i < vec->size - 1; i++){
			vec->buffer[i] = vec->buffer[i + 1];
		}
		free(vec->buffer[--vec->size]);
		if(vec->size > 0 && vec->size == vec->capacity / 4){
			cvResize(vec, vec->capacity / 2);
		}
	}
}

void *cvGet(cVector *vec, unsigned int pos){
	if(pos < vec->size){
		return(vec->buffer[pos]);
	}
	return(NULL);
}

void cvSet(cVector *vec, unsigned int pos, void *data, unsigned int bytes){
	if(pos < vec->size){
		free(vec->buffer[pos]);
		vec->buffer[pos] = malloc(bytes);
		memcpy(vec->buffer[pos], data, bytes);
	}
}

size_t cvSize(cVector *vec){
	return(vec->size);
}

void cvClear(cVector *vec){
	if(vec->buffer != NULL){
		unsigned int i;
		for(i = 0; i < vec->size; i++){
			free(vec->buffer[i]);
		}
		free(vec->buffer);
	}
}
