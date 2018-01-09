#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "socketConnectionHandler.h"

#ifdef _WIN32
	signed char ssStartup();
	void ssCleanup();
#endif

typedef struct {
	int type;
	int protocol;
	socketConnectionHandler connectionHandler;
} socketServer;

// Socket functions shared by TCP and UDP sockets.
void ssReportError(const char *failedFunction, const int errorCode);
signed char ssInit(socketServer *server, const int type, const int protocol, const int argc, const char *argv[],
                   signed char (*ssLoadConfig)(char(*)[45], uint16_t*, const int, const char**));
socketHandle  *ssGetSocketHandle(const socketServer *server, const size_t socketID);
socketDetails *ssGetSocketDetails(const socketServer *server, const size_t socketID);
signed char ssSocketTimedOut(socketServer *server, const size_t socketID, const uint32_t currentTick);
void ssCheckTimeouts(socketServer *server, const uint32_t currentTick);

#endif
