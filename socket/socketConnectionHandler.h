#ifndef SOCKETCONNECTIONHANDLER_H
#define SOCKETCONNECTIONHANDLER_H

#include "socketShared.h"
#include "../shared/flags.h"
#include "../shared/return.h"
#include "../shared/qualifiers.h"
#include <stdint.h>
#include <stdlib.h>

// Various socketDetails status flags.
#define SOCK_NEW_DATA     0x01  // The socket has sent data between the previous and current updates.
#define SOCK_CONNECTED    0x02  // The socket has connected between the previous and current updates.
#define SOCK_DISCONNECTED 0x04  // The socket has disconnected between the previous and current updates.
#define SOCK_TIMED_OUT    0x08  // The socket has timed out between the previous and current updates.
#define SOCK_ERROR        0x10  // There has been an error with the socket between the previous and current updates.

typedef struct {
	size_t id;
	socketAddrLength addressSize;  // The size of the socket's address, in bytes.
	struct sockaddr_storage address;
	uint32_t lastUpdateTick;  // The tick that data was last received from the socket on.
	int lastBufferSize;       // The size of the last buffer received from the socket, in bytes.
	char lastBuffer[SOCK_MAX_BUFFER_SIZE];  // The last buffer received from the socket.
	flags_t flags;
} socketDetails;

typedef struct {
	size_t size;
	size_t capacity;
	size_t *idStack;  // Array of available socket IDs, accessed as a stack.
	size_t *idLinks;  // idLink[x] will return the position in handles and details of the socket with the id 'x'.
	socketHandle  *handles;  // Holds an array of struct pollfds, separate from details for ssHandleConnectionsTCP().
	socketDetails *details;  // Holds an array of socketDetails for both TCP and UDP.
} socketConnectionHandler;

return_t scdResize(socketConnectionHandler *const __RESTRICT__ scd, const size_t capacity);
return_t scdAddSocket(socketConnectionHandler *const __RESTRICT__ scd, const socketHandle *const __RESTRICT__ handle, const socketDetails *const __RESTRICT__ details);
return_t scdRemoveSocket(socketConnectionHandler *const __RESTRICT__ scd, const size_t socketID);
return_t scdInit(socketConnectionHandler *const __RESTRICT__ scd, const size_t capacity, const socketHandle *const __RESTRICT__ masterHandle, const socketDetails *const __RESTRICT__ masterDetails);
void scdDelete(socketConnectionHandler *const __RESTRICT__ scd);

#endif
