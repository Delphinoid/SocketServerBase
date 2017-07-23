#include "socketUDP.h"

void ssSendDataUDP(socketServer *server, socketDetails details, const char *msg){
	if(sendto(ssGetSocketHandle(server, 0)->fd, msg, strlen(msg), 0, (struct sockaddr *)&details.address, details.bytes) < 0){
		ssReportError("sendto()", lastErrorID);
	}
}

void ssDisconnectSocketUDP(socketServer *server, size_t socketID){
	scdRemoveSocket(&server->connectionHandler, socketID);
}

void ssHandleConnectionsUDP(socketServer *server, uint32_t currentTick,
                            void (*ssHandleConnectUDP)(socketServer*, socketHandle*, socketDetails*),
                            void (*ssHandleBufferUDP)(socketServer*, socketDetails),
                            void (*ssHandleDisconnectUDP)(socketServer*, socketDetails, char),
                            unsigned char flags){

	// Keep receiving data while the buffer is not empty
	do{

		// Create ssSocket struct for the socket we are receiving data from
		socketDetails clientDetails;
		clientDetails.id = 0;
		clientDetails.bytes = sizeof(clientDetails.address);

		// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
		server->recvBytes = recvfrom(ssGetSocketHandle(server, 0)->fd, server->lastBuffer, SOCK_MAX_BUFFER_SIZE, 0,
		                             (struct sockaddr *)&clientDetails.address, &clientDetails.bytes);

		// Check if anything was received
		if(server->recvBytes > 0){

			// If the SOCK_ABSTRACT flag was specified, it's a little complex
			if((flags & SOCK_ABSTRACT_HANDLE) > 0){

				// Check if socket exists, and while we're at it disconnect any sockets that have timed out
				size_t i;
				for(i = 1; i < server->connectionHandler.size; i++){

					size_t oldSize = server->connectionHandler.size;
					socketDetails currentDetails = server->connectionHandler.details[i];

					// Disconnect the socket at index i if it has timed out
					if((flags & SOCK_MANAGE_TIMEOUTS) > 0 && ssSocketTimedOut(server, currentDetails.id, currentTick)){
						(*ssHandleDisconnectUDP)(server, currentDetails, SOCK_TIMED_OUT);

					// Check if the addresses are the same for the two sockets (includes port)
					}else if(clientDetails.id == 0 && memcmp(&clientDetails.address, &currentDetails.address, clientDetails.bytes)){
						clientDetails.id = currentDetails.id;
						if((flags & SOCK_MANAGE_TIMEOUTS) == 0){
							break;
						}
					}

					// If some sockets where shuffled (e.g. due to disconnects), reposition i
					if(currentDetails.id != server->connectionHandler.details[i].id){
						if(server->connectionHandler.idLinks[currentDetails.id] != 0){
							i = server->connectionHandler.idLinks[currentDetails.id];
						}else{
							if(oldSize > server->connectionHandler.size){
								// Handled oddly because we're using unsigned integers
								// oldSize is now the difference in size between oldSize and server->connectionHandler.size
								oldSize -= server->connectionHandler.size;
								if(oldSize >= i){
									i = 1;
								}else{
									// May jump too far back, but this is a rare case anyway
									i -= oldSize;
								}
							}
						}
					}

				}

				// If the socket was not found (we currently do not have a session with it), add it to the connection handler
				if(clientDetails.id == 0){
					socketHandle clientHandle;
					clientHandle.fd = server->connectionHandler.handles[0].fd;
					(*ssHandleConnectUDP)(server, &clientHandle, &clientDetails);
				}

			}

			// Do something with the received data
			(*ssHandleBufferUDP)(server, clientDetails);

		}else{
			// Error was encountered, abort the loop
			int tempLastErrorID = lastErrorID;
			// Don't bother reporting the error if it's EWOULDBLOCK or ECONNRESET, as it can be ignored here
			if(tempLastErrorID != EWOULDBLOCK && tempLastErrorID != ECONNRESET){
				ssReportError("recvfrom()", tempLastErrorID);
			}
			break;
		}

	}while((flags & SOCK_READ_FULL_QUEUE) > 0);

}

void ssShutdownUDP(socketServer *server){
	scdDelete(&server->connectionHandler);
}
