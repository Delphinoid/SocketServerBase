#include "socketTCP.h"
#include <stdio.h>

void ssSendDataTCP(socketServer *server, unsigned int socketID, const char *msg){
	if(send(svGet(&server->connectedSockets, socketID), msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
	}
}

void ssHandleBufferTCP(socketServer *server, unsigned int senderID){
	ssSendDataTCP(server, senderID, "Message received over TCP successfully. You should get this!");
}

void ssHandleConnectionsTCP(socketServer *server){

	/* Empties and refills socketSet, as select() may have modified it */
	FD_ZERO(&server->socketSet);
	FD_SET(server->masterSocketTCP, &server->socketSet);
	unsigned int d;
	for(d = 0; d < server->connectedSockets.size; d++){
		FD_SET(svGet(&server->connectedSockets, d), &server->socketSet);
	}


	// Checks which sockets have changed state, and removes the ones that haven't from socketSet
	int changedSockets = select(0, &server->socketSet, NULL, NULL, NULL);

	if(changedSockets != SOCKET_ERROR){  // If select() did not return SOCKET_ERROR (-1), all is fine

		if(changedSockets > 0){  // Only continue if there are sockets that have changed state

			/* If the master socket has changed state, there is an incoming connection. Accept the connection if the socket is valid */
			if(FD_ISSET(server->masterSocketTCP, &server->socketSet)){

				SOCKET clientSocket = accept(server->masterSocketTCP, NULL, NULL);

				if(clientSocket != INVALID_SOCKET){
					FD_SET(clientSocket, &server->socketSet);
					svPush(&server->connectedSockets, clientSocket);
					printf("Accepted connection from socket #%i.\n", clientSocket);
				}else{
					ssReportError("accept()", lastErrorID);
				}

			}

			/* Receive data from and send data to connected sockets */
			for(d = 0; d < server->connectedSockets.size; d++){  // Loop through each connected socket

				if(FD_ISSET(svGet(&server->connectedSockets, d), &server->socketSet)){  // Check if the socket actually has changed state

					// Receives up to MAX_TCP_BUFFER_SIZE bytes of data from a client socket and stores it in lastBufferTCP
					memset(server->lastBufferTCP, 0, MAX_TCP_BUFFER_SIZE);  // Reset lastBufferTCP
					server->recvBytesTCP = recv(svGet(&server->connectedSockets, d), server->lastBufferTCP, MAX_TCP_BUFFER_SIZE, 0);

					if(server->recvBytesTCP == -1){  // Error encountered, disconnect problematic socket

						ssReportError("recv()", lastErrorID);
						printf("Closing connection with socket #%i.\n\n", svGet(&server->connectedSockets, d));
						ssDisconnectSocket(server, d);
						d--;

					}else if(server->recvBytesTCP == 0){  // If the buffer is empty, the connection has closed

						printf("Socket #%i has disconnected, closing connection.\n", svGet(&server->connectedSockets, d));
						ssDisconnectSocket(server, d);
						d--;

					}else{  // Data received

						ssHandleBufferTCP(server, d);  // Do something with the received data

					}

				}

			}

		}

	}else{

		ssReportError("select()", lastErrorID);

	}

}
