#ifndef SOCKETSHARED_H
#define SOCKETSHARED_H

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

#include "cVector.h"
#include <stdint.h>

typedef struct {

	char ip[40];  // Max 40 characters instead of 16 to accommodate for IPv6 in the future (last is '\0')
	uint16_t port;
	SOCKET masterSocket;  // Host's socket object

} socketShared;

// Socket functions shared by TCP and UDP sockets
int inet_pton(int af, const char *src, char *dst);
void ssReportError(const char *failedFunction, int errorCode);
void ssShutdownShared();

#endif
