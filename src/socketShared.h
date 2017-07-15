#ifndef SOCKETSHARED_H
#define SOCKETSHARED_H

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include "win_pollfd.h"
	#define lastErrorID WSAGetLastError()
	#define WINSOCK_VERSION MAKEWORD(2, 2)
	#define socketHandle struct pollfd
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
	#define socketHandle struct pollfd
#endif

#endif
