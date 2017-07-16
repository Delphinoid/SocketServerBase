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
	int recvBytesTCP;  // Length of the last message recieved over TCP
	int recvBytesUDP;  // Length of the last message recieved over UDP
	char lastBufferTCP[SOCK_MAX_BUFFER_SIZE];  // Last message received from a client over TCP. Buffers are capped at MAX_BUFFER_SIZE bytes
	char lastBufferUDP[SOCK_MAX_BUFFER_SIZE];  // Last message received from a client over UDP. Buffers are capped at MAX_BUFFER_SIZE bytes
	socketConnectionHandler connectionHandler;
} socketServer;

// Socket functions shared by TCP and UDP sockets
void ssReportError(const char *failedFunction, int errorCode);
unsigned char ssInit(socketServer *server, int type, int protocol, const int argc, const char *argv[],
                     unsigned char (*ssLoadConfig)(char(*)[45], uint16_t*, const int, const char**));
socketHandle  *ssGetSocketHandle(socketServer *server, size_t socketID);
socketDetails *ssGetSocketDetails(socketServer *server, size_t socketID);
unsigned char ssSocketTimedOut(socketServer *server, size_t socketID, uint32_t currentTick);
void ssCheckTimeouts(socketServer *server, uint32_t currentTick);

#endif
