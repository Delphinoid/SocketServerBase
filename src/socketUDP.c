#include "socketUDP.h"
#include <stdio.h>

int ssInitShared(socketShared *hostData, int hostType, int hostProtocol);

int ssInitUDP(socketUDP *server, const int argc, const char *argv[], int (*loadConfig)(char*, uint16_t*, const int, const char**)){

	printf("Initializing UDP socket...\n");

	/* Initialize server IP and port, then load the server config */
	server->hostData.ip[0] = '\0';
	server->hostData.port = DEFAULT_PORT_UDP;
	if(loadConfig != NULL){
		loadConfig(server->hostData.ip, &server->hostData.port, argc, argv);
	}

	/* Initialize master socket */
	server->hostData.af = ssGetAddressFamily(server->hostData.ip);
	if(server->hostData.af == -1){
		server->hostData.af = DEFAULT_ADDRESS_FAMILY_UDP;
	}
	if(!ssInitShared(&server->hostData, SOCK_DGRAM, IPPROTO_UDP)){
		return 0;
	}

	/* Initialize connectedSockets vector */
	cvInit(&server->connectedSockets, 1);

	printf("UDP socket successfully initialized on %s:%u.\n\n", server->hostData.ip, server->hostData.port);
	return 1;

}

void ssSendDataUDP(socketUDP *server, struct sockaddr *client, const char *msg){
	if(sendto(server->hostData.masterSocket, msg, strlen(msg), 0, client, sizeof(struct sockaddr_in6)) < 0){  // Use the size of sockaddr_in6
		ssReportError("sendto()", lastErrorID);
	}
}

void ssHandleConnectionsUDP(socketUDP *server,
                            void (*handleBuffer)(socketUDP*, struct sockaddr*),
                            void (*handleTimeout)(socketUDP*)){

	// Create sockaddr struct for the socket we are receiving data from
	struct sockaddr socketDetails;
	int socketDetailsSize = sizeof(struct sockaddr_in6);  // Use the size of sockaddr_in6

	// Receives up to MAX_BUFFER_SIZE_UDP bytes of data from a client socket and stores it in lastBuffer
	memset(server->lastBuffer, 0, MAX_BUFFER_SIZE_UDP);  // Reset lastBuffer
	server->recvBytes = recvfrom(server->hostData.masterSocket, server->lastBuffer, MAX_BUFFER_SIZE_UDP, 0, &socketDetails, &socketDetailsSize);

	if(server->recvBytes == -1){  // Error encountered
		if(lastErrorID != 10054){  // Don't bother reporting the error if it's WSAECONNRESET, as it can be ignored here
			ssReportError("recvfrom()", lastErrorID);
		}
	}else if(server->recvBytes > 0){  // Data received successfully
		(*handleBuffer)(server, &socketDetails);  // Do something with the received data
	}

}

void ssShutdownUDP(socketUDP *server){
	cvClear(&server->connectedSockets);
}
