#include "socketUDP.h"

signed char ssSendDataUDP(const socketServer *server, const socketDetails *details, const char *msg){
	if(sendto(ssGetSocketHandle(server, 0)->fd, msg, strlen(msg), 0, (struct sockaddr *)&details->address, details->addressSize) < 0){
		ssReportError("sendto()", lastErrorID);
		return 0;
	}
	return 1;
}

signed char ssHandleConnectionsUDP(socketServer *server, uint32_t currentTick, const unsigned char flags){

	// Keep receiving data while the buffer is not empty.
	do{

		// Create ssSocket struct for the socket we are receiving data from.
		socketDetails clientDetails;
		clientDetails.id = 0;
		clientDetails.addressSize = sizeof(clientDetails.address);

		// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer.
		clientDetails.lastBufferSize = recvfrom(ssGetSocketHandle(server, 0)->fd, clientDetails.lastBuffer, SOCK_MAX_BUFFER_SIZE, 0,
		                                        (struct sockaddr *)&clientDetails.address, &clientDetails.addressSize);

		// Check if anything was received.
		if(clientDetails.lastBufferSize > 0){

			// Check if socket exists, and while we're at it disconnect any sockets that have timed out.
			size_t i;
			for(i = 1; i < server->connectionHandler.size; ++i){

				// Check if the addresses are the same for the two sockets (includes port).
				if(clientDetails.id == 0 && memcmp(&clientDetails.address, &server->connectionHandler.details[i].address, clientDetails.addressSize)){
					clientDetails.id = server->connectionHandler.details[i].id;
					if((flags & SOCK_MANAGE_TIMEOUTS) == 0){
						break;
					}

				// Disconnect the socket at index i if it has timed out.
				}else if((flags & SOCK_MANAGE_TIMEOUTS) > 0 && ssSocketTimedOut(server, server->connectionHandler.details[i].id, currentTick)){
					server->connectionHandler.details[i].flags |= SOCK_TIMED_OUT;

				}

			}

			// Check if i is within correct bounds.
			if(i < server->connectionHandler.capacity){

				// If the socket was not found (we currently do not have a session with it), add it to the connection handler.
				if(clientDetails.id == 0){
					socketHandle clientHandle;
					clientHandle.fd = server->connectionHandler.handles[0].fd;
					const size_t id = scdAddSocket(&server->connectionHandler, &clientHandle, &clientDetails);
					ssGetSocketDetails(server, id)->flags |= SOCK_CONNECTED;
				}

				// Copy over the last buffer.
				server->connectionHandler.details[i].lastBufferSize = clientDetails.lastBufferSize;
				memcpy(server->connectionHandler.details[i].lastBuffer, clientDetails.lastBuffer, clientDetails.lastBufferSize);

				// Do something with the received data.
				server->connectionHandler.details[i].flags |= SOCK_NEW_DATA;

			}else{
				// Server is full.
			}

		}else{
			// Error was encountered, abort the loop.
			const int tempLastErrorID = lastErrorID;
			// Don't bother reporting the error if it's EWOULDBLOCK or ECONNRESET, as it can be ignored here.
			if(tempLastErrorID != EWOULDBLOCK && tempLastErrorID != ECONNRESET){
				ssReportError("recvfrom()", tempLastErrorID);
				return 0;
			}
			break;
		}

	}while((flags & SOCK_READ_FULL_QUEUE) > 0);

	return 1;

}

void ssShutdownUDP(socketServer *server){
	scdDelete(&server->connectionHandler);
}
