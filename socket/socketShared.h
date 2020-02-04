#ifndef SOCKETSHARED_H
#define SOCKETSHARED_H

#include "../settings/socketSettings.h"

#ifndef SOCK_DEFAULT_ADDRESS_FAMILY
	#define SOCK_DEFAULT_ADDRESS_FAMILY AF_INET6
#endif
#ifndef SOCK_DEFAULT_PORT
	#define SOCK_DEFAULT_PORT 7249
#endif
#ifndef SOCK_MAX_BUFFER_SIZE
	#define SOCK_MAX_BUFFER_SIZE 4096
#endif
#ifndef SOCK_POLL_TIMEOUT
	#define SOCK_POLL_TIMEOUT 0
#endif
#ifndef SOCK_CONNECTION_TIMEOUT
	#define SOCK_CONNECTION_TIMEOUT 60000
#endif
#ifndef SOCK_MAX_SOCKETS
	#define SOCK_MAX_SOCKETS 257
#endif
#ifdef SOCK_USE_POLL
	#define SOCK_POLL_FUNC "poll()"
#else
	#define SOCK_POLL_FUNC "select()"
#endif

#undef FD_SETSIZE
#undef __FD_SETSIZE
#define FD_SETSIZE SOCK_MAX_SOCKETS
#define __FD_SETSIZE SOCK_MAX_SOCKETS

// Flags passed into ssHandleConnections functions.
#define SOCK_UDP             0x01  // Currently no longer used.
#define SOCK_TCP             0x02  // Currently no longer used.
#define SOCK_VERBOSE         0x04  // Currently no longer used.
#define SOCK_MANAGE_TIMEOUTS 0x08
#define SOCK_ABSTRACT_HANDLE 0x10  // Currently no longer used.
#define SOCK_READ_FULL_QUEUE 0x20

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
	#define socketclose(x) closesocket(x)
	struct pollfd {
		int fd;
		short events;
		short revents;
	};
	#define socketHandle struct pollfd
	#define socketAddrLength int
	int inet_pton(int af, const char *src, char *dst);
	const char *inet_ntop(int af, const void *src, char *dst, size_t size);
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>
	#define lastErrorID errno
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define socketclose(x) close(x)
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
			int fd;
			short events;
			short revents;
		};
	#endif
	#define socketHandle struct pollfd
	#define socketAddrLength socklen_t
#endif

int pollFunc(socketHandle *ufds, size_t nfds, int timeout);

#endif
