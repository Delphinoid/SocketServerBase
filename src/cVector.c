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

void cvSetHelper(cVector *vec, unsigned int pos, void *data, enum type dataType, unsigned int length){
	switch(dataType){
		case CHAR_T:
			vec->buffer[pos] = malloc(length * sizeof(char));
			memcpy(vec->buffer[pos], data, length * sizeof(char));
		break;
		case INT_T:
			vec->buffer[pos] = malloc(length * sizeof(int));
			memcpy(vec->buffer[pos], data, length * sizeof(int));
		break;
		case LONG_T:
			vec->buffer[pos] = malloc(length * sizeof(long));
			memcpy(vec->buffer[pos], data, length * sizeof(long));
		break;
		case FLOAT_T:
			vec->buffer[pos] = malloc(length * sizeof(float));
			memcpy(vec->buffer[pos], data, length * sizeof(float));
		break;
		case DOUBLE_T:
			vec->buffer[pos] = malloc(length * sizeof(double));
			memcpy(vec->buffer[pos], data, length * sizeof(double));
		break;
		case LONG_DOUBLE_T:
			vec->buffer[pos] = malloc(length * sizeof(long double));
			memcpy(vec->buffer[pos], data, length * sizeof(long double));
		break;
		default:
			vec->buffer[pos] = malloc(length * sizeof(*data));
			memcpy(vec->buffer[pos], data, length * sizeof(*data));
		break;
	}
}

void cvPush(cVector *vec, void *data, enum type dataType, unsigned int length){
	if(vec->size == vec->capacity){
		cvResize(vec, vec->capacity * 2);
	}
	cvSetHelper(vec, vec->size, data, dataType, length);
	vec->size++;
}

void cvPop(cVector *vec){
	free(vec->buffer[--vec->size]);
}

void cvInsert(cVector *vec, unsigned int pos, void *data, enum type dataType, unsigned int length){
	if(pos < vec->size){
		if(vec->size == vec->capacity){
			cvResize(vec, vec->capacity * 2);
		}
		unsigned int i;
		for(i = pos + 1; i < vec->size; i++){
			vec->buffer[i] = vec->buffer[i - 1];
		}
		free(vec->buffer[pos]);
		cvSetHelper(vec, pos, data, dataType, length);
		vec->size++;
	}else{
		cvPush(vec, data, dataType, length);
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

void cvSet(cVector *vec, unsigned int pos, void *data, enum type dataType, unsigned int length){
	if(pos < vec->size){
		free(vec->buffer[pos]);
		cvSetHelper(vec, pos, data, dataType, length);
	}
}

size_t cvSize(cVector *vec){
	return(vec->size);
}

void cvClear(cVector *vec){
	unsigned int i;
	for(i = 0; i < vec->size; i++){
		free(vec->buffer[i]);
	}
	free(vec->buffer);
}
