#include "socketTCP.h"
#include <stdio.h>

void ssSendDataTCP(socketHandle *clientHandle, const char *msg){
	if(send(clientHandle->fd, msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
	}
}

void ssDisconnectSocketTCP(socketServer *server, size_t socketID){
	closesocket(ssGetSocketHandle(server, socketID)->fd);
	scdRemoveSocket(&server->connectionHandler, socketID);
}

void ssHandleConnectionsTCP(socketServer *server, uint32_t currentTick,
                            void (*ssHandleConnectTCP)(socketServer*, socketHandle*, socketDetails*),
                            void (*ssHandleBufferTCP)(socketServer*, socketDetails),
                            void (*ssHandleDisconnectTCP)(socketServer*, socketDetails, char),
                            unsigned char flags){

	int changedSockets = pollFunc(server->connectionHandler.handles, server->connectionHandler.size, SOCK_POLL_TIMEOUT);

	if(changedSockets != SOCKET_ERROR){  // If pollFunc() did not return SOCKET_ERROR (-1), all is fine

		/* Handle state changes for each socket */
		// Loop through each connected socket
		size_t i;
		for(i = 0; ((flags & SOCK_MANAGE_TIMEOUTS) > 0 || changedSockets > 0) && i < server->connectionHandler.size; i++){

			socketHandle  currentHandle  = server->connectionHandler.handles[i];
			socketDetails currentDetails = server->connectionHandler.details[i];

			// Disconnect the socket at index i if it has timed out
			if((flags & SOCK_MANAGE_TIMEOUTS) > 0 && ssSocketTimedOut(server, currentDetails.id, currentTick)){
				(*ssHandleDisconnectTCP)(server, currentDetails, SOCK_TIMED_OUT);

			// Disconnect the socket at index i if a hang up was detected
			}else if((currentHandle.revents & POLLHUP) > 0){
				(*ssHandleDisconnectTCP)(server, currentDetails, SOCK_DISCONNECTED);
				changedSockets--;

			// Check if any revents flags have been set
			}else if(currentHandle.revents != 0){

				// Set the last update tick
				currentDetails.lastUpdateTick = currentTick;

				// Master socket has changed state, accept incoming sockets
				if(currentDetails.id == 0){

					socketHandle  clientHandle;
					socketDetails clientDetails;
					clientDetails.bytes = sizeof(clientDetails.address);

					clientHandle.fd = accept(ssGetSocketHandle(server, 0)->fd, (struct sockaddr *)&clientDetails.address, &clientDetails.bytes);

					// Check if accept() was successful
					if(clientHandle.fd != INVALID_SOCKET){
						clientHandle.events = POLLIN;
						clientHandle.revents = 0;
						clientDetails.lastUpdateTick = currentTick;
						(*ssHandleConnectTCP)(server, &clientHandle, &clientDetails);
					}else{
						ssReportError("accept()", lastErrorID);
					}

				// A client has changed state, receive incoming data
				}else{

					// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
					server->recvBytes = recv(currentHandle.fd, server->lastBuffer, SOCK_MAX_BUFFER_SIZE, 0);

					if(server->recvBytes == -1){
						// Error encountered, disconnect problematic socket
						ssReportError("recv()", lastErrorID);
						(*ssHandleDisconnectTCP)(server, currentDetails, SOCK_ERROR);
					}else if(server->recvBytes == 0){
						// If the buffer is empty, the connection has closed
						(*ssHandleDisconnectTCP)(server, currentDetails, SOCK_DISCONNECTED);
					}else{
						// Data received
						(*ssHandleBufferTCP)(server, currentDetails);
					}

				}

				// Clear the revents flags
				currentHandle.revents = 0;
				changedSockets--;

			}

			// If some sockets where shuffled (e.g. due to disconnects), reposition i
			if(currentDetails.id != server->connectionHandler.details[i].id){
				if(server->connectionHandler.idLinks[currentDetails.id] != 0){
					i = server->connectionHandler.idLinks[currentDetails.id];
				}else{
					i--;
				}
			}

		}

	}else{

		ssReportError(SOCK_POLL_FUNC, lastErrorID);

	}

}

void ssShutdownTCP(socketServer *server){
	size_t i = server->connectionHandler.size;
	while(i > 0){
		closesocket(server->connectionHandler.handles[--i].fd);
	}
	scdDelete(&server->connectionHandler);
}
