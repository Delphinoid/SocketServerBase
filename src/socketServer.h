#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "cVector.h"
#include <stdint.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define lastErrorID WSAGetLastError()
	#define WINSOCK_VERSION MAKEWORD(2, 2)
	unsigned char ssStartup();
	void ssCleanup();
#else
	#include <sys/socket.h>
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

	char ip[40];  // Max 40 characters instead of 16 to accommodate for IPv6 in the future (last is '\0')
	int af;  // Host address family. Either AF_INET (IPv4) or AF_INET6 (IPv6)
	uint16_t port;
	int type;
	int protocol;
	SOCKET masterSocket;  // Host's socket handle
	cVector connectedSockets;  // Holds a dynamic array of socket handles for TCP, possibly sockaddr_storages for UDP
	int recvBytes;  // Length of the last message recieved
	char lastBuffer[MAX_BUFFER_SIZE];  // Last message received from a client. Buffers are capped at MAX_BUFFER_SIZE bytes

} socketServer;

// Socket functions shared by TCP and UDP sockets
int  inet_pton(int af, const char *src, char *dst);
int  ssGetAddressFamily(const char *ip);
void ssReportError(const char *failedFunction, int errorCode);
unsigned char ssInit(socketServer *server, int type, int protocol, const int argc, const char *argv[],
                     unsigned char (*loadConfig)(socketServer*, const int, const char**));

#endif
