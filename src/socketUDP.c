#include "socketUDP.h"

void ssSendDataUDP(socketServer *server, socketDetails details, const char *msg){
	if(sendto(ssGetSocketHandle(server, 0)->fd, msg, strlen(msg), 0, (struct sockaddr *)&details.address, sizeof(details.address)) < 0){
		ssReportError("sendto()", lastErrorID);
	}
}

void ssHandleConnectionsUDP(socketServer *server,
                            void (*ssHandleBufferUDP)(socketServer*, socketDetails),
                            void (*ssHandleTimeoutUDP)(socketServer*)){

	// Create ssSocket struct for the socket we are receiving data from
	socketDetails clientDetails;
	int socketDetailsSize = sizeof(clientDetails.address);

	// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
	memset(server->lastBuffer, 0, MAX_BUFFER_SIZE);  // Reset lastBuffer
	server->recvBytes = recvfrom(ssGetSocketHandle(server, 0)->fd, server->lastBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&clientDetails.address, &socketDetailsSize);

	if(server->recvBytes == -1){  // Error encountered
		if(lastErrorID != 10054){  // Don't bother reporting the error if it's WSAECONNRESET, as it can be ignored here
			ssReportError("recvfrom()", lastErrorID);
		}
	}else if(server->recvBytes > 0){  // Data received successfully
		(*ssHandleBufferUDP)(server, clientDetails);  // Do something with the received data
	}

}

void ssShutdownUDP(socketServer *server){
	scdDelete(&server->connectionHandler);
}
