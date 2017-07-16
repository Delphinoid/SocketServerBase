#include "socketTCP.h"
#include <stdio.h>

void ssSendDataTCP(socketServer *server, int handle, const char *msg){
	if(send(handle, msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
	}
}

void ssDisconnectSocketTCP(socketServer *server, size_t socketID){
	closesocket(ssGetSocketHandle(server, socketID)->fd);
	scdRemoveSocket(&server->connectionHandler, socketID);
}

void ssHandleConnectionsTCP(socketServer *server, uint32_t currentTick,
                            void (*ssHandleConnectTCP)(socketServer*, socketHandle*, socketDetails*),
                            void (*ssHandleBufferTCP)(socketServer*, size_t),
                            void (*ssHandleDisconnectTCP)(socketServer*, size_t, int),
                            unsigned char flags){

	int changedSockets = pollFunc(server->connectionHandler.handles, server->connectionHandler.size, SOCK_POLL_TIMEOUT);

	if(changedSockets != SOCKET_ERROR){  // If pollFunc() did not return SOCKET_ERROR (-1), all is fine

		/* Handle state changes for each socket */
		// Loop through each connected socket
		size_t i;
		for(i = 0; ((flags & SOCK_MANAGE_TIMEOUTS) > 0 || changedSockets > 0) && i < server->connectionHandler.size; i++){

			// Disconnect the socket at index i if it has timed out
			if((flags & SOCK_MANAGE_TIMEOUTS) > 0 && ssSocketTimedOut(server, server->connectionHandler.details[i].id, currentTick)){
				(*ssHandleDisconnectTCP)(server, server->connectionHandler.details[i].id, SOCK_TIMED_OUT);
				i--;

			// Check if any revents flags have been set
			}else if(server->connectionHandler.handles[i].revents != 0){

				// Clear the revents flags
				server->connectionHandler.handles[i].revents = 0;
				// Set the last update tick
				server->connectionHandler.details[i].lastUpdateTick = currentTick;
				// Store the current number of sockets connected for later on
				size_t oldSize = server->connectionHandler.size;

				// Master socket has changed state, accept incoming sockets
				if(i == 0){

					socketHandle  clientHandle;
					socketDetails clientDetails;
					memset(&clientDetails, 0, sizeof(clientDetails));
					int socketDetailsSize = sizeof(clientDetails.address);

					clientHandle.fd = accept(ssGetSocketHandle(server, 0)->fd, (struct sockaddr *)&clientDetails.address, &socketDetailsSize);

					// Check if accept() was successful
					if(clientHandle.fd != INVALID_SOCKET){
						clientHandle.events = POLLWRNORM | POLLWRBAND;
						clientHandle.revents = 0;
						clientDetails.lastUpdateTick = currentTick;
						(*ssHandleConnectTCP)(server, &clientHandle, &clientDetails);
					}else{
						ssReportError("accept()", lastErrorID);
					}

				// A client has changed state, receive incoming data
				}else{

					// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
					server->recvBytesTCP = recv(server->connectionHandler.handles[i].fd, server->lastBufferTCP, SOCK_MAX_BUFFER_SIZE, 0);

					// Error encountered, disconnect problematic socket
					if(server->recvBytesTCP == -1){

						ssReportError("recv()", lastErrorID);
						(*ssHandleDisconnectTCP)(server, server->connectionHandler.details[i].id, SOCK_ERROR);


					// If the buffer is empty, the connection has closed
					}else if(server->recvBytesTCP == 0){

						(*ssHandleDisconnectTCP)(server, server->connectionHandler.details[i].id, SOCK_DISCONNECTED);

					// Data received
					}else{

						(*ssHandleBufferTCP)(server, server->connectionHandler.details[i].id);

					}

				}

				// Jump back if some sockets disconnected. It may go too far back,
				// but that shouldn't matter much
				if(oldSize > server->connectionHandler.size){
					// Handled oddly because we're using unsigned integers
					size_t difference = oldSize - server->connectionHandler.size;
					if(difference >= i){
						i = 1;
					}else{
						i -= oldSize - server->connectionHandler.size;
					}
				}

				changedSockets--;

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
