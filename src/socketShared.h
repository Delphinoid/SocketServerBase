#ifndef SOCKETSHARED_H
#define SOCKETSHARED_H

#include "socketSettings.h"

#ifndef SOCK_DEFAULT_ADDRESS_FAMILY
	#define SOCK_DEFAULT_ADDRESS_FAMILY AF_INET6
#endif
#ifndef SOCK_DEFAULT_PORT
	#define SOCK_DEFAULT_PORT 7249
#endif
#ifndef SOCK_MAX_BUFFER_SIZE
	#define SOCK_MAX_BUFFER_SIZE 1024
#endif
#ifndef SOCK_POLL_TIMEOUT
	#define SOCK_POLL_TIMEOUT 0
#endif
#ifndef SOCK_CONNECTION_TIMEOUT
	#define SOCK_CONNECTION_TIMEOUT 60000
#endif
#ifndef SOCK_MAX_SOCKETS
	#define SOCK_MAX_SOCKETS 201
#endif
#undef FD_SETSIZE
#undef __FD_SETSIZE
#define FD_SETSIZE SOCK_MAX_SOCKETS
#define __FD_SETSIZE SOCK_MAX_SOCKETS

#ifdef SOCK_USE_POLL
	#define SOCK_POLL_FUNC "poll()"
#else
	#define SOCK_POLL_FUNC "select()"
#endif

// Flags passed into ssHandleConnections functions
#define SOCK_UDP             0x01  // Currently not used
#define SOCK_TCP             0x02  // Currently not used
#define SOCK_VERBOSE         0x04  // Currently not used
#define SOCK_MANAGE_TIMEOUTS 0x08
#define SOCK_ABSTRACT_HANDLE 0x10
#define SOCK_READ_FULL_QUEUE 0x20

// Types of disconnects passed into ssHandleDisconnect functions
#define SOCK_ERROR -1
#define SOCK_DISCONNECTED 0
#define SOCK_TIMED_OUT 1

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define lastErrorID WSAGetLastError()
	#define WINSOCK_VERSION MAKEWORD(2, 2)
	#define EWOULDBLOCK WSAEWOULDBLOCK
	#define ECONNRESET WSAECONNRESET
	#define POLLHUP    0x002
	#define POLLOUT    0x010
	#define POLLIN     0x100
	#define POLLPRI    0x200
	int ioctl(int, long, unsigned long*);
	struct pollfd {
		SOCKET fd;
		short events;
		short revents;
	};
	#define socketHandle struct pollfd
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#define lastErrorID errno
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define SOCKET int
	#ifdef SOCK_USE_POLL
		#include <sys/poll.h>
	#else
		#define POLLIN     0x0001
		#define POLLPRI    0x0002
		#define POLLOUT    0x0004
		#define POLLERR    0x0008
		#define POLLHUP    0x0010
		#define POLLNVAL   0x0020
		#define POLLRDNORM 0x0040
		#define POLLRDBAND 0x0080
		#define POLLWRNORM 0x0100
		#define POLLWRBAND 0x0200
		struct pollfd {
			SOCKET fd;
			short events;
			short revents;
		};
	#endif
	#define socketHandle struct pollfd
#endif

int pollFunc(socketHandle*, size_t, int);

#endif
