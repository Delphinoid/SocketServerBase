#include "socketConnectionHandler.h"

unsigned char scdResize(socketConnectionHandler *scd, size_t capacity){

	if(capacity > scd->capacity){

		// Allocate memory for the ID stack
		void *buffer = realloc(scd->idStack, capacity*sizeof(size_t));
		if(buffer != NULL){
			scd->idStack = buffer;
		}else{
			return 0;
		}
		// Allocate memory for the used ID array
		buffer = realloc(scd->idLinks, capacity*sizeof(size_t));
		if(buffer != NULL){
			scd->idLinks = buffer;
		}else{
			free(scd->idStack);
			return 0;
		}
		// Allocate memory for the socket handle array
		buffer = realloc(scd->handles, capacity*sizeof(socketHandle));
		if(buffer != NULL){
			scd->handles = buffer;
		}else{
			free(scd->idStack);
			free(scd->idLinks);
			return 0;
		}
		// Allocate memory for the socket details array
		buffer = realloc(scd->details, capacity*sizeof(socketDetails));
		if(buffer != NULL){
			scd->details = buffer;
		}else{
			free(scd->idStack);
			free(scd->idLinks);
			free(scd->handles);
			return 0;
		}

		// Initialize the ID stack and used ID array, starting from the old capacity
		size_t i = scd->capacity;
		while(i < capacity){
			scd->idStack[i] = i;
			scd->idLinks[i] = 0;
			i++;
		}

		scd->capacity = capacity;
		return 1;

	}

	return 0;

}

unsigned char scdAddSocket(socketConnectionHandler *scd, socketHandle *handle, socketDetails *details){

	if(scd->size < scd->capacity){

		// Link the handle and details arrays to the used ID array
		scd->handles[scd->size] = *handle;
		details->id = scd->idStack[scd->size];
		scd->details[scd->size] = *details;
		scd->idLinks[scd->idStack[scd->size]] = scd->size;
		scd->idStack[scd->size] = 0;
		scd->size++;

		return 1;

	}

	return 0;

}

unsigned char scdRemoveSocket(socketConnectionHandler *scd, size_t socketID){

	// Don't touch element 0 (the master socket)
	if(socketID > 0){

		scd->size--;

		// Shift everything after this element over and adjust their links
		size_t i;
		for(i = scd->idLinks[socketID]; i < scd->size; i++){
			scd->handles[i] = scd->handles[i+1];
			scd->details[i] = scd->details[i+1];
			scd->idLinks[scd->details[i].id]--;
		}

		// Free the socket ID
		scd->idStack[scd->size] = socketID;
		scd->idLinks[socketID] = 0;
		return 1;

	}

	return 0;

}

unsigned char scdInit(socketConnectionHandler *scd, size_t capacity, socketHandle *masterHandle, socketDetails *masterDetails){

	// Initialize everything
	scd->size = 0;
	scd->capacity = 0;
	scd->idStack = NULL;
	scd->idLinks = NULL;
	scd->handles = NULL;
	scd->details = NULL;
	if(scdResize(scd, capacity) == 0){
		return 0;
	}

	// Add the master socket
	return scdAddSocket(scd, masterHandle, masterDetails);

}

void scdDelete(socketConnectionHandler *scd){
	if(scd->idStack != NULL){
		free(scd->idStack);
	}
	if(scd->idLinks != NULL){
		free(scd->idLinks);
	}
	if(scd->handles != NULL){
		free(scd->handles);
	}
	if(scd->details != NULL){
		free(scd->details);
	}
}
