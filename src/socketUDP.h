#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketShared.h"

#ifndef MAX_BUFFER_SIZE_UDP
	#define MAX_BUFFER_SIZE_UDP 2048
#endif
#ifndef DEFAULT_PORT_UDP
	#define DEFAULT_PORT_UDP 7249
#endif

typedef struct {

	socketShared hostData;
	cVector connectedSockets;  // Intended to be an array of sockaddr_ins that have sent data over UDP; whether or not this is used is up to the implementation
	int recvBytes;  // Length of the last buffer recieved over UDP
	char lastBuffer[MAX_BUFFER_SIZE_UDP];  // Last buffer ("message") received from a client over UDP. Buffers are capped at MAX_BUFFER_SIZE_UDP bytes

} socketUDP;

int  ssInitUDP(socketUDP *server, const int argc, const char *argv[], int (*loadConfig)(char*, uint16_t*, const int, const char**));
void ssSendDataUDP(socketUDP *server, struct sockaddr_in *client, const char *msg);
void ssHandleConnectionsUDP(socketUDP *server, void (*handleBuffer)(socketUDP*, struct sockaddr_in*), void (*handleDisconnect)(socketUDP*));
void ssShutdownUDP(socketUDP *server);

#endif
