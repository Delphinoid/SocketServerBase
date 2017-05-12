#include "socketUDP.h"
#include <stdio.h>

void ssSendDataUDP(socketServer *server, struct sockaddr_storage *client, const char *msg){
	if(sendto(server->masterSocket, msg, strlen(msg), 0, (struct sockaddr*)client, sizeof(struct sockaddr_storage)) < 0){
		ssReportError("sendto()", lastErrorID);
	}
}

void ssHandleConnectionsUDP(socketServer *server,
                            void (*handleBuffer)(socketServer*, struct sockaddr_storage*),
                            void (*handleTimeout)(socketServer*)){

	// Create sockaddr struct for the socket we are receiving data from
	struct sockaddr_storage socketDetails;
	int socketDetailsSize = sizeof(struct sockaddr_storage);

	// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
	memset(server->lastBuffer, 0, MAX_BUFFER_SIZE);  // Reset lastBuffer
	server->recvBytes = recvfrom(server->masterSocket, server->lastBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&socketDetails, &socketDetailsSize);

	if(server->recvBytes == -1){  // Error encountered
		if(lastErrorID != 10054){  // Don't bother reporting the error if it's WSAECONNRESET, as it can be ignored here
			ssReportError("recvfrom()", lastErrorID);
		}
	}else if(server->recvBytes > 0){  // Data received successfully
		(*handleBuffer)(server, &socketDetails);  // Do something with the received data
	}

}

void ssShutdownUDP(socketServer *server){
	cvClear(&server->connectedSockets);
}
