#include "socketTCP.h"
#include <stdio.h>

int ssInitTCP(socketTCP *server, const int argc, const char *argv[], int (*loadConfig)(char*, uint16_t*, const int, const char**)){

	printf("Initializing TCP socket...\n");


	/* Initialize server IP and port, then load the server config */
	server->hostData.ip[0] = '\0';
	server->hostData.port = DEFAULT_PORT_TCP;

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
	int addressFamily = ssGetAddressFamily(server->hostData.ip);  // Get the address family of the host address
	server->hostData.masterSocket = socket(addressFamily, SOCK_STREAM, IPPROTO_TCP);  // TCP socket
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

	// Bind the address to the TCP socket
	if(bind(server->hostData.masterSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR){  // If bind() failed, abort
		ssReportError("bind()", lastErrorID);
		#ifdef _WIN32
			WSACleanup();
		#endif
		return 0;
	}


	/* Change the TCP master socket's state to "listen" so it will start listening for incoming connections from sockets */
	if(listen(server->hostData.masterSocket, SOMAXCONN) == SOCKET_ERROR){  // SOMAXCONN = automatically choose maximum number of pending connections, different across systems
		ssReportError("listen()", lastErrorID);
		#ifdef _WIN32
			WSACleanup();
		#endif
		return 0;
	}


	/* Initialize connectedSockets vector */
	cvInit(&server->connectedSockets, 1);


	printf("TCP socket successfully initialized on %s:%u.\n\n", server->hostData.ip, server->hostData.port);
	return 1;

}

void ssSendDataTCP(socketTCP *server, unsigned int socketID, const char *msg){
	if(send(*((SOCKET*)cvGet(&server->connectedSockets, socketID)), msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
	}
}

void ssHandleConnectionsTCP(socketTCP *server,
                            void (*handleBuffer)(socketTCP*, unsigned int),
                            void (*handleDisconnect)(socketTCP*, unsigned int)){

	/* Empties and refills socketSet, as select() may have modified it */
	FD_ZERO(&server->socketSet);
	FD_SET(server->hostData.masterSocket, &server->socketSet);
	unsigned int d;
	for(d = 0; d < server->connectedSockets.size; d++){
		FD_SET(*((SOCKET*)cvGet(&server->connectedSockets, d)), &server->socketSet);
	}


	// Checks which sockets have changed state, and removes the ones that haven't from socketSet
	int changedSockets = select(0, &server->socketSet, NULL, NULL, NULL);

	if(changedSockets != SOCKET_ERROR){  // If select() did not return SOCKET_ERROR (-1), all is fine

		if(changedSockets > 0){  // Only continue if there are sockets that have changed state

			/* If the master socket has changed state, there is an incoming connection. Accept the connection if the socket is valid */
			if(FD_ISSET(server->hostData.masterSocket, &server->socketSet)){

				SOCKET clientSocket = accept(server->hostData.masterSocket, NULL, NULL);

				if(clientSocket != INVALID_SOCKET){
					FD_SET(clientSocket, &server->socketSet);
					cvPush(&server->connectedSockets, &clientSocket, INT_T, 1);
					printf("Accepted TCP connection from socket #%i.\n", clientSocket);
				}else{
					ssReportError("accept()", lastErrorID);
				}

			}

			/* Receive data from and send data to connected sockets */
			for(d = 0; d < server->connectedSockets.size; d++){  // Loop through each connected socket

				if(FD_ISSET(*((SOCKET*)cvGet(&server->connectedSockets, d)), &server->socketSet)){  // Check if the socket actually has changed state

					// Receives up to MAX_TCP_BUFFER_SIZE bytes of data from a client socket and stores it in lastBufferTCP
					memset(server->lastBuffer, 0, MAX_BUFFER_SIZE_TCP);  // Reset lastBufferTCP
					server->recvBytes = recv(*((SOCKET*)cvGet(&server->connectedSockets, d)), server->lastBuffer, MAX_BUFFER_SIZE_TCP, 0);

					if(server->recvBytes == -1){  // Error encountered, disconnect problematic socket

						ssReportError("recv()", lastErrorID);
						(*handleDisconnect)(server, d);
						closesocket(*((SOCKET*)cvGet(&server->connectedSockets, d)));
						FD_CLR(*((SOCKET*)cvGet(&server->connectedSockets, d)), &server->socketSet);
						cvErase(&server->connectedSockets, d);
						d--;

					}else if(server->recvBytes == 0){  // If the buffer is empty, the connection has closed

						(*handleDisconnect)(server, d);
						closesocket(*((SOCKET*)cvGet(&server->connectedSockets, d)));
						FD_CLR(*((SOCKET*)cvGet(&server->connectedSockets, d)), &server->socketSet);
						cvErase(&server->connectedSockets, d);
						d--;

					}else{  // Data received

						(*handleBuffer)(server, d);  // Do something with the received data

					}

				}

			}

		}

	}else{

		ssReportError("select()", lastErrorID);

	}

}

void ssShutdownTCP(socketTCP *server){
	unsigned int d;
	for(d = 0; d < server->connectedSockets.size; d++){
		closesocket(*((SOCKET*)cvGet(&server->connectedSockets, d)));
	}
	cvClear(&server->connectedSockets);
	FD_ZERO(&server->socketSet);
	ssShutdownShared();
}
