#include "socketUDP.h"
#include <stdio.h>

int socketDetailsSize = 0;

void ssSendDataUDP(socketServer *server, struct sockaddr_in *client, const char *msg){
	if(sendto(server->masterSocketUDP, msg, strlen(msg), 0, (struct sockaddr*)client, sizeof(*client)) < 0){
		ssReportError("sendto()", lastErrorID);
	}
}

void ssHandleBufferUDP(socketServer *server, struct sockaddr_in *sender){
	if(ssSocketHasConnected(server, sender)){
		ssSendDataUDP(server, sender, "Message received over UDP successfully. I hope you get this!");
	}else{
		printf("Data received from an unknown source. It will be ignored...\n");
	}
}

void ssHandleConnectionsUDP(socketServer *server){

	// Create sockaddr struct for the socket we are receiving data from
	struct sockaddr_in socketDetails;
	int socketDetailsSize = sizeof(socketDetails);

	// Receives up to MAX_UDP_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
	memset(server->lastBufferUDP, 0, MAX_UDP_BUFFER_SIZE);  // Reset lastBufferUDP
	server->recvBytesUDP = recvfrom(server->masterSocketUDP, server->lastBufferUDP, MAX_UDP_BUFFER_SIZE, 0, (struct sockaddr*)&socketDetails, &socketDetailsSize);

	if(server->recvBytesUDP == -1){  // Error encountered
		ssReportError("recvfrom()", lastErrorID);
	}else if(server->recvBytesUDP > 0){  // Data received successfully
		ssHandleBufferUDP(server, &socketDetails);  // Do something with the received data
	}

}
