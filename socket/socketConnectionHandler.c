#include "socketConnectionHandler.h"
#include "../memory/memoryManager.h"

__FORCE_INLINE__ return_t sdTimedOut(const socketDetails *const __RESTRICT__ details, const uint32_t currentTick){
	return currentTick - details->lastTick >= SOCKET_CONNECTION_TIMEOUT;
}

return_t scInit(socketConnectionHandler *const __RESTRICT__ sc, const size_t capacity, const socketHandle *const __RESTRICT__ masterHandle, const socketDetails *const __RESTRICT__ masterDetails){

	socketDetails *details;
	socketHandle *handle;
	const socketHandle *handleLast;

	// Initialize socketDetails.
	void *memory = memAllocate(capacity * sizeof(socketDetails));
	if(memory == NULL){
		return -1;
	}
	sc->details = memory;
	details = memory;
	sc->detailsLast = details-1;

	// Initialize socketHandles.
	memory = memAllocate(capacity * sizeof(socketHandle));
	if(memory == NULL){
		return -1;
	}
	sc->handles = memory;
	handle = memory;
	sc->handleLast = handle-1;

	// Initialize the fd array.
	handleLast = &((socketHandle *)memory)[capacity];
	while(handle < handleLast){
		handle->fd = (intptr_t)details;
		details->handle = NULL;
		++handle; ++details;
	}

	sc->capacity = capacity;
	sc->nfds = 0;

	// Add the master socket.
	scAddSocket(sc, masterHandle, masterDetails);
	return 1;

}

socketDetails *scAddSocket(socketConnectionHandler *const __RESTRICT__ sc, const socketHandle *const __RESTRICT__ handle, const socketDetails *const __RESTRICT__ details){

	if(sc->nfds >= sc->capacity){
		return NULL;
	}else{

		// The file descriptor stores a pointer
		// to its corresponding details buffer.
		socketHandle *const newHandle = ++sc->handleLast;
		socketDetails *const newDetails = (socketDetails *)newHandle->fd;

		*newHandle = *handle;
		newDetails->handle = newHandle;
		newDetails->addressSize = details->addressSize;
		newDetails->address = details->address;
		newDetails->lastTick = details->lastTick;
		newDetails->lastBufferSize = 0;
		newDetails->lastBuffer[0] = '\0';
		newDetails->flags = details->flags;

		sc->detailsLast = newDetails;
		++sc->nfds;

		return newDetails;

	}

}

return_t scRemoveSocket(socketConnectionHandler *const __RESTRICT__ sc, socketDetails *details){

	// Don't touch the master socket.
	if(details == scMasterDetails(sc)){
		return 0;
	}

	// Move the last handle to fill in the gap.
	*details->handle = *sc->handleLast;
	// Make the now-empty last handle point to
	// its new (empty) corresponding details.
	sc->handleLast->fd = (intptr_t)details;
	sc->detailsLast->handle = details->handle;
	// Shift the last handle back.
	--sc->handleLast;

	// These details are no longer in use.
	details->handle = NULL;

	--sc->nfds;
	return 1;

}

void scDelete(socketConnectionHandler *sc){

	socketHandle *handle = sc->handles;
	const socketHandle *const handleLast = &handle[sc->nfds];

	while(handle < handleLast){
		socketclose(handle->fd);
		++handle;
	}

	memFree(sc->handles);
	memFree(sc->details);

}
