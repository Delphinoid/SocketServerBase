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
                            void (*ssHandleBufferUDP)(socketServer*, void*),
                            void (*ssHandleDisconnectUDP)(socketServer*, size_t, char),
                            unsigned char flags){

	// Keep receiving data while the buffer is not empty
	while(1){

		// Create ssSocket struct for the socket we are receiving data from
		socketDetails clientDetails;
		clientDetails.bytes = sizeof(clientDetails.address);

		// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
		server->recvBytes = recvfrom(ssGetSocketHandle(server, 0)->fd, server->lastBuffer, SOCK_MAX_BUFFER_SIZE, 0,
		                             (struct sockaddr *)&clientDetails.address, &clientDetails.bytes);

		// Check if anything was received
		if(server->recvBytes > 0){

			// If the SOCK_ABSTRACT flag was specified, it's a little complex
			if((flags & SOCK_ABSTRACT_HANDLE) > 0){
				// Check if socket exists, and while we're at it disconnect any sockets that have timed out
				size_t socketID = 0;
				size_t i;
				for(i = 1; ((flags & SOCK_MANAGE_TIMEOUTS) > 0 || socketID == 0) && i < server->connectionHandler.size; i++){

					// Disconnect the socket at index i if it has timed out
					if((flags & SOCK_MANAGE_TIMEOUTS) > 0 && ssSocketTimedOut(server, server->connectionHandler.details[i].id, currentTick)){
						(*ssHandleDisconnectUDP)(server, server->connectionHandler.details[i].id, SOCK_TIMED_OUT);
						i--;

					// Check if the addresses are the same for the two sockets (includes port)
					}else if(memcmp(&clientDetails.address, &server->connectionHandler.details[i].address, clientDetails.bytes)){
						socketID = server->connectionHandler.details[i].id;
					}

				}

				// If the socket was not found (we currently do not have a session with it), add it to the connection handler
				if(socketID == 0){
					socketHandle clientHandle;
					clientHandle.fd = server->connectionHandler.handles[0].fd;
					(*ssHandleConnectUDP)(server, &clientHandle, &clientDetails);
					socketID = clientDetails.id;
				}

				// Do something with the received data
				(*ssHandleBufferUDP)(server, &socketID);


			// If the SOCK_ABSTRACT flag was not specified, just send the socket details into ssHandleBufferUDP()
			}else{
				// Do something with the received data
				(*ssHandleBufferUDP)(server, &clientDetails);
			}

		// Error was encountered, abort the loop
		}else{
			int tempLastErrorID = lastErrorID;
			// Don't bother reporting the error if it's EWOULDBLOCK or ECONNRESET, as it can be ignored here
			if(tempLastErrorID != EWOULDBLOCK && tempLastErrorID != ECONNRESET){
				ssReportError("recvfrom()", tempLastErrorID);
			}
			break;
		}

	}

}

void ssShutdownUDP(socketServer *server){
	scdDelete(&server->connectionHandler);
}
