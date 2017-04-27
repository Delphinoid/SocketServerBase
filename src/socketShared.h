#ifndef SOCKETSHARED_H
#define SOCKETSHARED_H

#include "cVector.h"
#include <stdint.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define lastErrorID WSAGetLastError()
	#define WINSOCK_VERSION MAKEWORD(2, 2)
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#define lastErrorID errno
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	typedef int SOCKET;
#endif

typedef struct {

	char ip[40];  // Max 40 characters instead of 16 to accommodate for IPv6 in the future (last is '\0')
	int af;  // Host address family. Either AF_INET (IPv4) or AF_INET6 (IPv6)
	uint16_t port;
	SOCKET masterSocket;  // Host's socket object

} socketShared;

// Socket functions shared by TCP and UDP sockets
int  inet_pton(int af, const char *src, char *dst);
int  ssGetAddressFamily(const char *ip);
void ssReportError(const char *failedFunction, int errorCode);
void ssShutdownShared();

#endif
