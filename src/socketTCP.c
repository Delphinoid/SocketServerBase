#include "socketTCP.h"
#include <stdio.h>

void ssSendDataTCP(socketServer *server, size_t socketID, const char *msg){
	if(send(ssGetSocketHandle(server, socketID)->fd, msg, strlen(msg) + 1, 0) < 0){
		ssReportError("send()", lastErrorID);
	}
}

void ssDisconnectSocketTCP(socketServer *server, size_t socketID){
	closesocket(ssGetSocketHandle(server, socketID)->fd);
	scdRemoveSocket(&server->connectionHandler, socketID);
}

void ssHandleConnectionsTCP(socketServer *server,
                            void (*ssHandleBufferTCP)(socketServer*, size_t),
                            void (*ssHandleDisconnectTCP)(socketServer*, size_t)){
	#ifdef _WIN32

	#else
	int changedSockets = poll(server->connectionHandler.handles, server->connectionHandler.idHandler.size, 0);

	if(changedSockets != SOCKET_ERROR){  // If poll() did not return SOCKET_ERROR (-1), all is fine

		/* Handle state changes for each socket */
		size_t i;
		for(i = 0; changedSockets > 0 && i < server->connectionHandler.size; i++){  // Loop through each connected socket

			// Check if POLLIN flag is set
			if(server->connectionHandler.handles[i].revents == POLLIN){

				// Clear the revents flags
				server->connectionHandler.handles[i].revents = 0;
				// Store the current number of sockets connected for later on
				size_t oldSize = server->connectionHandler.size;

				// Master socket has changed state, accept incoming sockets
				if(i == 0){

					socketHandle  clientHandle;
					socketDetails clientDetails;
					int socketDetailsSize = sizeof(clientDetails.address);

					// Keep going until all clients are accepted or an error occurs
					while(1){

						clientHandle.fd = accept(ssGetSocketHandle(server, 0)->fd, (struct sockaddr *)&clientDetails.address, &socketDetailsSize);

						// Check if accept() was successful
						if(clientHandle.fd != INVALID_SOCKET){
							clientHandle.events = POLLIN;
							scdAddSocket(server->connectionHandler, clientHandle, clientDetails);
							printf("Accepted TCP connection from socket #%i.\n", clientHandle.fd);
						}else{
							// Finished accepting sockets. If errno isn't set to EWOULDBLOCK, an error has occurred
							if(errno != EWOULDBLOCK){
								ssReportError("accept()", lastErrorID);
							}
							break;
						}

					}

				// A client has changed state, receive data
				}else{

					// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
					server->recvBytes = recv(ssGetSocketHandle(server, 0)->fd, server->lastBuffer, MAX_BUFFER_SIZE, 0);

					// Error encountered, disconnect problematic socket
					if(server->recvBytes == -1){

						ssReportError("recv()", lastErrorID);
						(*ssHandleDisconnectTCP)(server, server->connectionHandler.details[i].id);

					// If the buffer is empty, the connection has closed
					}else if(server->recvBytes == 0){

						(*ssHandleDisconnectTCP)(server, server->connectionHandler.details[i].id);

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
					if(difference > i){
						i = 1;
					}else{
						i -= oldSize - server->connectionHandler.size;
					}
				}

				// Since we found a socket that has changed state, modify changedSockets
				// in an attempt to end the loop sooner
				changedSockets--;

			}

		}

	}else{

		ssReportError("select()", lastErrorID);

	}
	#endif

}

void ssShutdownTCP(socketServer *server){
	size_t i = server->connectionHandler.size;
	while(i > 0){
		closesocket(server->connectionHandler.handles[--i].fd);
	}
	scdDelete(&server->connectionHandler);
}
