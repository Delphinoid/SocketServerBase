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
#ifdef SOCKET_DEBUG
void ssReportError(const char *const __RESTRICT__ failedFunction, const int errorCode);
#endif
return_t ssInit(socketServer *const __RESTRICT__ server, const int type, const int protocol, void *args, return_t (*ssLoadConfig)(char(*)[45], uint16_t*, void*));
socketHandle *ssGetSocketHandle(const socketServer *const __RESTRICT__ server, const size_t socketID);
void ssCheckTimeouts(socketConnectionHandler *const __RESTRICT__ sc, const uint32_t currentTick);

#endif
