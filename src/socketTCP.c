#include "socketTCP.h"
#include <stdio.h>

unsigned char ssSendDataTCP(const socketHandle *clientHandle, const char *msg){
	if(send(clientHandle->fd, msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
		return 0;
	}
	return 1;
}

unsigned char ssDisconnectSocketTCP(socketServer *server, const size_t socketID){
	socketclose(ssGetSocketHandle(server, socketID)->fd);
	return scdRemoveSocket(&server->connectionHandler, socketID);
}

unsigned char ssHandleConnectionsTCP(socketServer *server, const uint32_t currentTick, const unsigned char flags){

	int changedSockets = pollFunc(server->connectionHandler.handles, server->connectionHandler.size, SOCK_POLL_TIMEOUT);

	// If pollFunc() returned SOCKET_ERROR (-1), return.
	if(changedSockets == SOCKET_ERROR){
		ssReportError(SOCK_POLL_FUNC, lastErrorID);
		return 0;
	}

	/* Handle state changes for each socket. */
	// Loop through each connected socket.
	size_t i;
	for(i = 0; ((flags & SOCK_MANAGE_TIMEOUTS) > 0 || changedSockets > 0) && i < server->connectionHandler.size; ++i){

		// Disconnect the socket at index i if a hang up was detected.
		if((server->connectionHandler.handles[i].revents & POLLHUP) > 0){
			server->connectionHandler.details[i].flags |= SOCK_DISCONNECTED;
			socketclose(ssGetSocketHandle(server, server->connectionHandler.details[i].id)->fd);
			--changedSockets;

		// Check if any revents flags have been set.
		}else if(server->connectionHandler.handles[i].revents != 0){

			// Set the last update tick.
			server->connectionHandler.details[i].lastUpdateTick = currentTick;

			// Master socket has changed state, accept incoming sockets.
			if(server->connectionHandler.details[i].id == 0){

				socketHandle  clientHandle;
				socketDetails clientDetails;
				clientDetails.addressSize = sizeof(clientDetails.address);

				clientHandle.fd = accept(ssGetSocketHandle(server, 0)->fd, (struct sockaddr *)&clientDetails.address, &clientDetails.addressSize);

				// Check if accept() was successful.
				if(clientHandle.fd != INVALID_SOCKET){
					clientHandle.events = POLLIN;
					clientHandle.revents = 0;
					clientDetails.lastUpdateTick = currentTick;
					const size_t id = scdAddSocket(&server->connectionHandler, &clientHandle, &clientDetails);
					if(id){
						ssGetSocketDetails(server, id)->flags |= SOCK_CONNECTED;
					}else{
						// Server is full.
					}
				}else{
					ssReportError("accept()", lastErrorID);
					return 0;
				}

			// A client has changed state, receive incoming data.
			}else{

				// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer.
				server->connectionHandler.details[i].lastBufferSize = recv(server->connectionHandler.handles[i].fd, server->connectionHandler.details[i].lastBuffer, SOCK_MAX_BUFFER_SIZE, 0);

				if(server->connectionHandler.details[i].lastBufferSize == -1){
					// Error encountered, disconnect problematic socket.
					ssReportError("recv()", lastErrorID);
					socketclose(ssGetSocketHandle(server, server->connectionHandler.details[i].id)->fd);
					server->connectionHandler.details[i].flags |= SOCK_ERROR;
					return 0;
				}else if(server->connectionHandler.details[i].lastBufferSize == 0){
					// If the buffer is empty, the connection has closed.
					socketclose(ssGetSocketHandle(server, server->connectionHandler.details[i].id)->fd);
					server->connectionHandler.details[i].flags |= SOCK_DISCONNECTED;
				}else{
					// Data received.
					server->connectionHandler.details[i].flags |= SOCK_NEW_DATA;
				}

			}

			// Clear the revents flags.
			server->connectionHandler.handles[i].revents = 0;
			--changedSockets;

		// Disconnect the socket at index i if it has timed out.
		}else if((flags & SOCK_MANAGE_TIMEOUTS) > 0 && ssSocketTimedOut(server, server->connectionHandler.details[i].id, currentTick)){
			server->connectionHandler.details[i].flags |= SOCK_TIMED_OUT;

		}

	}

	return 1;

}

void ssShutdownTCP(socketServer *server){
	size_t i = server->connectionHandler.size;
	while(i > 0){
		socketclose(server->connectionHandler.handles[--i].fd);
	}
	scdDelete(&server->connectionHandler);
}
