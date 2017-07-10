#include "socketTCP.h"
#include <stdio.h>

size_t ssFindSocketTCP(socketServer *server, ssSocket *socket){
	size_t socketID = server->connectedSockets.size;
	while(socketID > 0){
		if(((ssSocket *)cvGet(&server->connectedSockets, --socketID))->handle.fd == socket->handle.fd){
			break;
		}
	}
	return socketID;
}

void ssSendDataTCP(socketServer *server, size_t socketID, const char *msg){
	if(send(((ssSocket *)cvGet(&server->connectedSockets, socketID))->handle.fd, msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
	}
}

void ssDisconnectSocketTCP(socketServer *server, size_t socketID){
	if(socketID != 0){
		closesocket(((ssSocket *)cvGet(&server->connectedSockets, socketID))->handle.fd);
		cvErase(&server->connectedSockets, socketID);
	}
}

void ssHandleConnectionsTCP(socketServer *server,
                            void (*ssHandleBufferTCP)(socketServer*, size_t),
                            void (*ssHandleDisconnectTCP)(socketServer*, size_t)){

	// Fill and perform operations on a temporary fd_set, as select() may modify it
	fd_set socketSet;
	FD_ZERO(&socketSet);
	size_t i;
	for(i = 0; i < server->connectedSockets.size; i++){
		FD_SET(((ssSocket *)cvGet(&server->connectedSockets, i))->handle.fd, &socketSet);
	}

	// Checks which sockets have changed state, and removes the ones that haven't from socketSet
	int changedSockets = select(0, &socketSet, NULL, NULL, NULL);

	if(changedSockets != SOCKET_ERROR){  // If select() did not return SOCKET_ERROR (-1), all is fine

		if(changedSockets > 0){  // Only continue if there are sockets that have changed state

			size_t i = 0;

			/* If the master socket has changed state, there is an incoming connection. Accept the connection if the socket is valid */
			if(socketSet.fd_array[i] == ssMasterSocket(server)->handle.fd){

				ssSocket client;
				int socketDetailsSize = sizeof(client.details);
				client.handle.fd = accept(ssMasterSocket(server)->handle.fd, (struct sockaddr *)&client.details, &socketDetailsSize);

				if(client.handle.fd != INVALID_SOCKET){
					cvPush(&server->connectedSockets, &client, sizeof(client));
					printf("Accepted TCP connection from socket #%i.\n", client.handle.fd);
				}else{
					ssReportError("accept()", lastErrorID);
				}

				i++;
				changedSockets--;

			}

			/* Receive data from and send data to connected sockets */
			for(; i < server->connectedSockets.size && changedSockets > 0; i++){  // Loop through each connected socket

				if(FD_ISSET(((ssSocket *)cvGet(&server->connectedSockets, i))->handle.fd, &socketSet)){

					// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
					memset(server->lastBuffer, 0, MAX_BUFFER_SIZE);  // Reset lastBuffer
					server->recvBytes = recv(socketSet.fd_array[i], server->lastBuffer, MAX_BUFFER_SIZE, 0);

					if(server->recvBytes == -1){  // Error encountered, disconnect problematic socket

						ssReportError("recv()", lastErrorID);
						(*ssHandleDisconnectTCP)(server, i);

					}else if(server->recvBytes == 0){  // If the buffer is empty, the connection has closed

						(*ssHandleDisconnectTCP)(server, i);

					}else{  // Data received

						(*ssHandleBufferTCP)(server, i);  // Do something with the received data

					}

					changedSockets--;

				}

			}

		}

	}else{

		ssReportError("select()", lastErrorID);

	}

}

void ssShutdownTCP(socketServer *server){
	size_t i = server->connectedSockets.size;
	while(i > 0){
		closesocket(*((SOCKET*)cvGet(&server->connectedSockets, --i)));
		cvPop(&server->connectedSockets);
	}
	cvClear(&server->connectedSockets);
}
