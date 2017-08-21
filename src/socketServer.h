#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "socketConnectionHandler.h"

#ifdef _WIN32
	unsigned char ssStartup();
	void ssCleanup();
#endif

typedef struct {
	int type;
	int protocol;
	int recvBytes;  // Length of the last message recieved
	char lastBuffer[SOCK_MAX_BUFFER_SIZE];  // Last message received from a client. Buffers are capped at MAX_BUFFER_SIZE bytes
	socketConnectionHandler connectionHandler;
} socketServer;

// Socket functions shared by TCP and UDP sockets
void ssReportError(const char *failedFunction, const int errorCode);
unsigned char ssInit(socketServer *server, const int type, const int protocol, const int argc, const char *argv[],
                     unsigned char (*ssLoadConfig)(char(*)[45], uint16_t*, const int, const char**));
socketHandle  *ssGetSocketHandle(const socketServer *server, const size_t socketID);
socketDetails *ssGetSocketDetails(const socketServer *server, const size_t socketID);
unsigned char ssSocketTimedOut(socketServer *server, const size_t socketID, const uint32_t currentTick);
void ssCheckTimeouts(socketServer *server, const uint32_t currentTick);

#endif
