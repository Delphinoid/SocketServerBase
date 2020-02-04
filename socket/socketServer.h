#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "socketConnectionHandler.h"

#ifdef _WIN32
	return_t ssStartup();
	void ssCleanup();
#else
	#define ssStartup() 1
	#define ssCleanup() ;
#endif

typedef struct {
	int type;
	int protocol;
	socketConnectionHandler connectionHandler;
} socketServer;

// Socket functions shared by TCP and UDP sockets.
void ssReportError(const char *const __RESTRICT__ failedFunction, const int errorCode);
return_t ssInit(socketServer *const __RESTRICT__ server, const int type, const int protocol, const int argc, char **argv, return_t (*ssLoadConfig)(char(*)[45], uint16_t*, const int, char**));
socketHandle  *ssGetSocketHandle(const socketServer *const __RESTRICT__ server, const size_t socketID);
socketDetails *ssGetSocketDetails(const socketServer *const __RESTRICT__ server, const size_t socketID);
return_t ssSocketTimedOut(socketServer *const __RESTRICT__ server, const size_t socketID, const uint32_t currentTick);
void ssCheckTimeouts(socketServer *const __RESTRICT__ server, const uint32_t currentTick);

#endif
