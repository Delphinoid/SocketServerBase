#include "socketUDP.h"
#include <stdio.h>

int ssInitUDP(socketUDP *server, const int argc, const char *argv[], int (*loadConfig)(char*, uint16_t*, const int, const char**)){

	printf("Initializing UDP socket...\n");


	/* Initialize server IP and port, then load the server config */
	server->hostData.ip[0] = '\0';
	server->hostData.port = DEFAULT_PORT_UDP;

	if(loadConfig != NULL){
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
	server->hostData.af = ssGetAddressFamily(server->hostData.ip);
	if(server->hostData.af == -1){
		server->hostData.af = DEFAULT_ADDRESS_FAMILY_UDP;
	}
	server->hostData.masterSocket = socket(server->hostData.af, SOCK_DGRAM, IPPROTO_UDP);  // TCP socket
	if(server->hostData.masterSocket == INVALID_SOCKET){  // If socket() failed, abort
		ssReportError("socket()", lastErrorID);
		#ifdef _WIN32
			WSACleanup();
		#endif
		return 0;
	}


	/* Bind the master socket to the host address */
	int bindStatus = SOCKET_ERROR;

	if(server->hostData.af == AF_INET){

		// Create the sockaddr_in structure using the master socket's address family and the supplied IP / port
		struct sockaddr_in serverAddress4;
		if(!inet_pton(server->hostData.af, server->hostData.ip, (char*)&(serverAddress4.sin_addr))){
			serverAddress4.sin_addr.s_addr = INADDR_ANY;
			strcpy(server->hostData.ip, "INADDR_ANY");
		}
		serverAddress4.sin_family = server->hostData.af;
		serverAddress4.sin_port = htons(server->hostData.port);  // htons() converts the port from big-endian to little-endian for sockaddr_in
		// Bind the address to the socket
		bindStatus = bind(server->hostData.masterSocket, (struct sockaddr*)&serverAddress4, sizeof(serverAddress4));

	}else if(server->hostData.af == AF_INET6){  // Address is IPv6

		// Create the sockaddr_in6 structure using the master socket's address family and the supplied IP / port
		struct sockaddr_in6 serverAddress6;
		memset(&serverAddress6, 0, sizeof(struct sockaddr_in6));  // Set everything in sockaddr_in6 to 0, as there are a number of fields we don't otherwise set
		if(!inet_pton(server->hostData.af, server->hostData.ip, (char*)&(serverAddress6.sin6_addr))){
			serverAddress6.sin6_addr = in6addr_any;
			strcpy(server->hostData.ip, "in6addr_any");
		}
		serverAddress6.sin6_family = server->hostData.af;
		serverAddress6.sin6_port = htons(server->hostData.port);  // htons() converts the port from big-endian to little-endian for sockaddr_in6
		// Bind the address to the socket
		bindStatus = bind(server->hostData.masterSocket, (struct sockaddr*)&serverAddress6, sizeof(serverAddress6));

	}

	// Check result of bind()
	if(bindStatus == SOCKET_ERROR){  // If bind() failed, abort
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
	ssShutdownShared();
}
