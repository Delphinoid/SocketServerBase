#include "socketUDP.h"
#include <stdio.h>

int ssInitUDP(socketUDP *server, const int argc, const char *argv[], int (*loadConfig)(char*, uint16_t*, const int, const char**)){

	printf("Initializing UDP socket...\n");


	/* Initialize server IP and port, then load the server config */
	server->hostData.ip[0] = '\0';
	server->hostData.port = DEFAULT_PORT_UDP;

	if(argc > 0 && loadConfig != NULL){  /** NEEDS FIXING **/
		loadConfig(server->hostData.ip, &server->hostData.port, argc, argv);
	}


	/* Specify the version of Winsock to use and initialize it */
	#ifdef _WIN32
		WSADATA wsaData;
		int initError = WSAStartup(WINSOCK_VERSION, &wsaData);

		if(initError != 0){  // If Winsock did not initialize correctly, abort
			ssReportError("WSAStartup()", initError);
			return 0;
		}
	#endif


	/* Create a socket prototype for the master socket */
	/*
	   socket(address family, type, protocol)
	   address family = AF_UNSPEC, which can be either IPv4 or IPv6
	   type = SOCK_STREAM, which uses TCP or SOCK_DGRAM, which uses UDP
	   protocol = IPPROTO_TCP, which specifies to use TCP or IPPROTO_TCP, which specifies to use UDP
	*/
	int addressFamily = ssGetAddressFamily(server->hostData.ip);
	server->hostData.masterSocket = socket(AF_UNSPEC, SOCK_DGRAM,  IPPROTO_UDP);  // UDP socket
	if(server->hostData.masterSocket == INVALID_SOCKET){  // If socket() failed, abort
		ssReportError("socket()", lastErrorID);
		#ifdef _WIN32
			WSACleanup();
		#endif
		return 0;
	}


	/* Bind the master socket to the host address */
	// Create the sockaddr_in structure using the master socket's address family and the supplied IP / port
	struct sockaddr_in serverAddress;
	// If the IP has been specified and the address family is valid, convert the IP from a string to the in_addr format for sockaddr_in
	if(strlen(server->hostData.ip) > 0 && addressFamily != AF_UNSPEC){
		inet_pton(addressFamily, server->hostData.ip, (char*)&(serverAddress.sin_addr));
	}else{  // Otherwise use all available addresses
		serverAddress.sin_addr.s_addr = INADDR_ANY;
		strcpy(server->hostData.ip, "INADDR_ANY");
		addressFamily = DEFAULT_ADDRESS_FAMILY;
	}
	serverAddress.sin_family = addressFamily;
	serverAddress.sin_port = htons(server->hostData.port);  // htons() converts the port from big-endian to little-endian for sockaddr_in

	// Bind the address to the UDP socket
	if(bind(server->hostData.masterSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR){  // If bind() failed, abort
		ssReportError("bind()", lastErrorID);
		#ifdef _WIN32
			WSACleanup();
		#endif
		return 0;
	}


	/* Initialize connectedSockets vector */
	cvInit(&server->connectedSockets, 1);


	printf("UDP socket successfully initialized on %s:%u.\n\n", server->hostData.ip, server->hostData.port);
	return 1;

}

void ssSendDataUDP(socketUDP *server, struct sockaddr_in *client, const char *msg){
	if(sendto(server->hostData.masterSocket, msg, strlen(msg), 0, (struct sockaddr*)client, sizeof(*client)) < 0){
		ssReportError("sendto()", lastErrorID);
	}
}

void ssHandleConnectionsUDP(socketUDP *server,
                            void (*handleBuffer)(socketUDP*, struct sockaddr_in*),
                            void (*handleTimeout)(socketUDP*)){

	// Create sockaddr struct for the socket we are receiving data from
	struct sockaddr_in socketDetails;
	int socketDetailsSize = sizeof(socketDetails);

	// Receives up to MAX_BUFFER_SIZE_UDP bytes of data from a client socket and stores it in lastBuffer
	memset(server->lastBuffer, 0, MAX_BUFFER_SIZE_UDP);  // Reset lastBuffer
	server->recvBytes = recvfrom(server->hostData.masterSocket, server->lastBuffer, MAX_BUFFER_SIZE_UDP, 0, (struct sockaddr*)&socketDetails, &socketDetailsSize);

	if(server->recvBytes == -1){  // Error encountered
		if(lastErrorID != 10054){  // Don't bother reporting the error if it's WSAECONNRESET, as it can be ignored here
			ssReportError("recvfrom()", lastErrorID);
		}
	}else if(server->recvBytes > 0){  // Data received successfully
		(*handleBuffer)(server, &socketDetails);  // Do something with the received data
	}

}

void ssShutdownUDP(socketUDP *server){
	ssShutdownShared();
}
