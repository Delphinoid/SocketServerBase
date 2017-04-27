#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketShared.h"

#ifndef DEFAULT_ADDRESS_FAMILY_TCP
	#define DEFAULT_ADDRESS_FAMILY_TCP AF_INET6
#endif
#ifndef DEFAULT_PORT_TCP
	#define DEFAULT_PORT_TCP 7249
#endif
#ifndef MAX_BUFFER_SIZE_TCP
	#define MAX_BUFFER_SIZE_TCP 1024
#endif

typedef struct {

	socketShared hostData;
	FD_SET socketSet;        // Set of sockets connected over TCP for select()
	cVector connectedSockets;  // Array of SOCKETs connected over TCP
	int recvBytes;  // Length of the last message recieved over TCP
	char lastBuffer[MAX_BUFFER_SIZE_TCP];  // Last message received from a client over TCP. Buffers are capped at MAX_BUFFER_SIZE_TCP bytes

} socketTCP;

int  ssInitTCP(socketTCP *server, const int argc, const char *argv[], int (*loadConfig)(char*, uint16_t*, const int, const char**));
void ssSendDataTCP(socketTCP *server, unsigned int clientID, const char *msg);
void ssHandleConnectionsTCP(socketTCP *server, void (*handleBuffer)(socketTCP*, unsigned int), void (*handleDisconnect)(socketTCP*, unsigned int));
void ssShutdownTCP(socketTCP *server);

#endif
