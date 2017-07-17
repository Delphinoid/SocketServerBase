#ifndef SOCKETCONNECTIONHANDLER_H
#define SOCKETCONNECTIONHANDLER_H

#include "socketShared.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	size_t id;
	uint32_t lastUpdateTick;
	struct sockaddr_storage address;
	int bytes;
} socketDetails;

typedef struct {
	size_t size;
	size_t capacity;
	size_t *idStack;  // Array of available socket IDs, accessed as a stack
	size_t *idLinks;  // idLink[x] will return the position in handles and details of the socket with the id 'x'
	socketHandle  *handles;  // Holds an array of struct pollfds, separate from details for ssHandleConnectionsTCP()
	socketDetails *details;  // Holds an array of socketDetails for both TCP and UDP
} socketConnectionHandler;

unsigned char scdResize(socketConnectionHandler *scd, size_t capacity);
unsigned char scdAddSocket(socketConnectionHandler *scd, socketHandle *handle, socketDetails *details);
unsigned char scdRemoveSocket(socketConnectionHandler *scd, size_t socketID);
unsigned char scdInit(socketConnectionHandler *scd, size_t capacity, socketHandle *masterHandle, socketDetails *masterDetails);
void scdDelete(socketConnectionHandler *scd);

#endif
