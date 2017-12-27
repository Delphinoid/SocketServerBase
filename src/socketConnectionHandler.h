#ifndef SOCKETCONNECTIONHANDLER_H
#define SOCKETCONNECTIONHANDLER_H

#include "socketShared.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	size_t id;
	struct sockaddr_storage address;
	uint32_t lastUpdateTick;  // The tick that data was last received from the socket on
	int lastBufferSize;       // How many bytes the last buffer received from the socket was
	char lastBuffer[SOCK_MAX_BUFFER_SIZE];  // The last buffer received from the socket
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

unsigned char scdResize(socketConnectionHandler *scd, const size_t capacity);
unsigned char scdAddSocket(socketConnectionHandler *scd, const socketHandle *handle, const socketDetails *details);
unsigned char scdRemoveSocket(socketConnectionHandler *scd, const size_t socketID);
unsigned char scdInit(socketConnectionHandler *scd, const size_t capacity, const socketHandle *masterHandle, const socketDetails *masterDetails);
void scdDelete(socketConnectionHandler *scd);

#endif
