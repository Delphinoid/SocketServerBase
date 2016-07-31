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

#define MAX_TCP_BUFFER_SIZE 1024
#define MAX_UDP_BUFFER_SIZE 2048

#define DEFAULT_PORT 7249

#define INIT_TCP 0
#define INIT_UDP 1
#define INIT_TCP_AND_UDP 2

#include "socketVector.h"
#include <stdint.h>

// Socket server typedef
typedef struct{

	char ip[40];  // Max 40 characters instead of 16 to accommodate for IPv6 in the future (last is '\0')
	uint16_t port;
	SOCKET masterSocketTCP;  // Host's socket object (for TCP connections)
	SOCKET masterSocketUDP;  // Host's socket object (for UDP connections)
	FD_SET socketSet;        // Set of sockets connected over TCP for select()
	socketVector connectedSockets;  // Array of sockets connected over TCP
	int recvBytesTCP;  // Length of the last buffer recieved over TCP
	int recvBytesUDP;  // Length of the last buffer recieved over UDP
	char lastBufferTCP[MAX_TCP_BUFFER_SIZE];  // Last buffer ("message") received from a client over TCP. Buffers are capped at MAX_TCP_BUFFER_SIZE bytes
	char lastBufferUDP[MAX_UDP_BUFFER_SIZE];  // Last buffer ("message") received from a client over UDP. Buffers are capped at MAX_UDP_BUFFER_SIZE bytes

} socketServer;

// Socket functions shared by TCP and UDP sockets
void ssReportError(const char *failedFunction, int errorCode);
int  ssLoadConfig(socketServer *server, const char *prgPath);
int  ssInit(socketServer *server, unsigned int type, const int argc, const char *argv[]);
int  ssSocketHasConnected(socketServer *server, struct sockaddr_in *socketDetails);  // Currently incomplete
void ssDisconnectSocket(socketServer *server, unsigned int socketID);
void ssShutdown(socketServer *server);

#endif
