#include "socketUDP.h"

size_t ssFindSocketUDP(socketServer *server, ssSocket *socket){
	size_t socketID = server->connectedSockets.size;
	while(socketID > 0){
		if(--socketID == socket->handle.fd){
			break;
		}
	}
	return socketID;
}

void ssSendDataUDP(socketServer *server, ssSocket *socket, const char *msg){
	if(sendto(ssMasterSocket(server)->handle.fd, msg, strlen(msg), 0, (struct sockaddr *)&socket->details, sizeof(socket->details)) < 0){
		ssReportError("sendto()", lastErrorID);
	}
}

void ssHandleConnectionsUDP(socketServer *server,
                            void (*ssHandleBufferUDP)(socketServer*, ssSocket*),
                            void (*ssHandleTimeoutUDP)(socketServer*)){

	// Create ssSocket struct for the socket we are receiving data from
	ssSocket client;
	int socketDetailsSize = sizeof(client.details);

	// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer
	memset(server->lastBuffer, 0, MAX_BUFFER_SIZE);  // Reset lastBuffer
	server->recvBytes = recvfrom(ssMasterSocket(server)->handle.fd, server->lastBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client.details, &socketDetailsSize);

	if(server->recvBytes == -1){  // Error encountered
		if(lastErrorID != 10054){  // Don't bother reporting the error if it's WSAECONNRESET, as it can be ignored here
			ssReportError("recvfrom()", lastErrorID);
		}
	}else if(server->recvBytes > 0){  // Data received successfully
		(*ssHandleBufferUDP)(server, &client);  // Do something with the received data
	}

}

void ssShutdownUDP(socketServer *server){
	cvClear(&server->connectedSockets);
}
