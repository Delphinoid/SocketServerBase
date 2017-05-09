#include "socketTCP.h"
#include <stdio.h>

int ssInitShared(socketShared *hostData, int hostType, int hostProtocol);

int ssInitTCP(socketTCP *server, const int argc, const char *argv[], int (*loadConfig)(char*, uint16_t*, const int, const char**)){

	printf("Initializing TCP socket...\n");

	/* Initialize server IP and port, then load the server config */
	server->hostData.ip[0] = '\0';
	server->hostData.port = DEFAULT_PORT_TCP;
	if(loadConfig != NULL){
		loadConfig(server->hostData.ip, &server->hostData.port, argc, argv);
	}

	/* Initialize master socket */
	server->hostData.af = ssGetAddressFamily(server->hostData.ip);
	if(server->hostData.af == -1){
		server->hostData.af = DEFAULT_ADDRESS_FAMILY_TCP;
	}
	if(!ssInitShared(&server->hostData, SOCK_STREAM, IPPROTO_TCP)){
		return 0;
	}

	/* Change the TCP master socket's state to "listen" so it will start listening for incoming connections from sockets */
	if(listen(server->hostData.masterSocket, SOMAXCONN) == SOCKET_ERROR){  // SOMAXCONN = automatically choose maximum number of pending connections, different across systems
		ssReportError("listen()", lastErrorID);
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
}
