#include "socketTCP.h"
#include <stdio.h>

void ssSendDataTCP(socketServer *server, size_t socketID, const char *msg){
	if(send(*((SOCKET*)cvGet(&server->connectedSockets, socketID)), msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
	}
}

void ssHandleConnectionsTCP(socketServer *server,
                            void (*handleBuffer)(socketServer*, size_t),
                            void (*handleDisconnect)(socketServer*, size_t)){

	// Fill and perform operations on a temporary FD_SET, as select() may modify it
	FD_SET socketSet;
	FD_ZERO(&socketSet);
	FD_SET(server->masterSocket, &socketSet);
	size_t i;
	for(i = 0; i < server->connectedSockets.size; i++){
		FD_SET(*((SOCKET*)cvGet(&server->connectedSockets, i)), &socketSet);
	}

	// Checks which sockets have changed state, and removes the ones that haven't from socketSet
	int changedSockets = select(0, &socketSet, NULL, NULL, NULL);

	if(changedSockets != SOCKET_ERROR){  // If select() did not return SOCKET_ERROR (-1), all is fine

		if(changedSockets > 0){  // Only continue if there are sockets that have changed state

			/* If the master socket has changed state, there is an incoming connection. Accept the connection if the socket is valid */
			if(FD_ISSET(server->masterSocket, &socketSet)){

				SOCKET clientSocket = accept(server->masterSocket, NULL, NULL);

				if(clientSocket != INVALID_SOCKET){
					FD_SET(clientSocket, &socketSet);
					cvPush(&server->connectedSockets, &clientSocket, sizeof(clientSocket));
					printf("Accepted TCP connection from socket #%i.\n", clientSocket);
				}else{
					ssReportError("accept()", lastErrorID);
				}

			}

			/* Receive data from and send data to connected sockets */
			for(i = 0; i < server->connectedSockets.size; i++){  // Loop through each connected socket

				if(FD_ISSET(*((SOCKET*)cvGet(&server->connectedSockets, i)), &socketSet)){  // Check if the socket actually has changed state

					// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
					memset(server->lastBuffer, 0, MAX_BUFFER_SIZE);  // Reset lastBuffer
					server->recvBytes = recv(*((SOCKET*)cvGet(&server->connectedSockets, i)), server->lastBuffer, MAX_BUFFER_SIZE, 0);

					if(server->recvBytes == -1){  // Error encountered, disconnect problematic socket

						ssReportError("recv()", lastErrorID);
						(*handleDisconnect)(server, i);
						closesocket(*((SOCKET*)cvGet(&server->connectedSockets, i)));
						FD_CLR(*((SOCKET*)cvGet(&server->connectedSockets, i)), &socketSet);
						cvErase(&server->connectedSockets, i);
						i--;

					}else if(server->recvBytes == 0){  // If the buffer is empty, the connection has closed

						(*handleDisconnect)(server, i);
						closesocket(*((SOCKET*)cvGet(&server->connectedSockets, i)));
						FD_CLR(*((SOCKET*)cvGet(&server->connectedSockets, i)), &socketSet);
						cvErase(&server->connectedSockets, i);
						i--;

					}else{  // Data received

						(*handleBuffer)(server, i);  // Do something with the received data

					}

				}

			}

		}

	}else{

		ssReportError("select()", lastErrorID);

	}

}

void ssShutdownTCP(socketServer *server){
	size_t i;
	for(i = 0; i < server->connectedSockets.size; i++){
		closesocket(*((SOCKET*)cvGet(&server->connectedSockets, i)));
	}
	cvClear(&server->connectedSockets);
}
