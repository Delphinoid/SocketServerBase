#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "socketConnectionHandler.h"

#ifdef _WIN32
	unsigned char ssStartup();
	void ssCleanup();
#endif

#ifndef DEFAULT_ADDRESS_FAMILY
	#define DEFAULT_ADDRESS_FAMILY AF_INET6
#endif
#ifndef DEFAULT_PORT
	#define DEFAULT_PORT 7249
#endif
#ifndef MAX_BUFFER_SIZE
	#define MAX_BUFFER_SIZE 1024
#endif
#ifndef MAX_CONNECTIONS
	#define MAX_CONNECTIONS 201
#endif

typedef struct {
	int type;
	int protocol;
	socketConnectionHandler connectionHandler;
	int recvBytes;  // Length of the last message recieved
	char lastBuffer[MAX_BUFFER_SIZE];  // Last message received from a client. Buffers are capped at MAX_BUFFER_SIZE bytes
} socketServer;

// Socket functions shared by TCP and UDP sockets
void ssReportError(const char *failedFunction, int errorCode);
unsigned char ssInit(socketServer *server, int type, int protocol, const int argc, const char *argv[],
                     unsigned char (*ssLoadConfig)(char(*)[45], uint16_t*, const int, const char**));
socketHandle  *ssGetSocketHandle(socketServer *server, size_t socketID);
socketDetails *ssGetSocketDetails(socketServer *server, size_t socketID);

#endif
