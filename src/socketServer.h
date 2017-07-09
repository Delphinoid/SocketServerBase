#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "cVector.h"
#include <stdint.h>

#ifdef MAX_SOCKETS
	#define FD_SETSIZE MAX_SOCKETS
#endif

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include "WSAPoll.h"
	#define lastErrorID WSAGetLastError()
	#define WINSOCK_VERSION MAKEWORD(2, 2)
	unsigned char ssStartup();
	void ssCleanup();
#else
	#include <sys/socket.h>
	#include <sys/poll.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#define lastErrorID errno
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define SOCKET int;
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

typedef struct {
	pollfd handle;
	struct sockaddr_storage details;
	uint32_t lastUpdate;
} ssSocket;

typedef struct {
	int type;
	int protocol;
	cVector connectedSockets;  // Holds a dynamic array of ssSockets for both TCP and UDP
	int recvBytes;  // Length of the last message recieved
	char lastBuffer[MAX_BUFFER_SIZE];  // Last message received from a client. Buffers are capped at MAX_BUFFER_SIZE bytes
} socketServer;

// Socket functions shared by TCP and UDP sockets
void ssReportError(const char *failedFunction, int errorCode);
ssSocket *ssMasterSocket(socketServer *server);
unsigned char ssInit(socketServer *server, int type, int protocol, const int argc, const char *argv[],
                     unsigned char (*ssLoadConfig)(char(*)[40], uint16_t*, const int, const char**));

#endif
