#include "socketTCP.h"
#include <string.h>
#include <stdio.h>

return_t ssSendDataTCP(const socketHandle *const __RESTRICT__ clientHandle, const char *const __RESTRICT__ msg){
	if(send(clientHandle->fd, msg, strlen(msg) + 1, 0) < 0){
		#ifdef SOCKET_DEBUG
		ssReportError("send()", lastErrorID);
		#endif
		return 0;
	}
	return 1;
}

return_t ssDisconnectSocketTCP(socketConnectionHandler *const __RESTRICT__ sc, socketDetails *clientDetails){
	socketclose(clientDetails->handle->fd);
	return scRemoveSocket(sc, clientDetails);
}

return_t ssHandleConnectionsTCP(socketConnectionHandler *const __RESTRICT__ sc, const uint32_t currentTick, const flags_t flags){

	socketDetails *i = sc->details;
	size_t j = sc->nfds;

	int changedSockets = pollFunc(sc->handles, j, SOCKET_POLL_TIMEOUT);

	// If pollFunc() returned SOCKET_ERROR (-1), return.
	if(changedSockets == SOCKET_ERROR){
		#ifdef SOCKET_DEBUG
		ssReportError(SOCKET_POLL_FUNC, lastErrorID);
		#endif
		return 0;
	}

	// Handle state changes for each socket.
	// Loop through each connected socket.
	while(j > 0){

		if((flagsAreUnset(flags, SOCKET_FLAGS_MANAGE_TIMEOUTS) && changedSockets <= 0)){
			break;
		}

		if(sdValid(i)){

			// Disconnect the current socket if a hang up was detected.
			if((i->handle->revents & POLLHUP) > 0){
				i->flags |= SOCKET_DETAILS_DISCONNECTED;
				socketclose(i->handle->fd);
				--changedSockets;

			// Check if any revents flags have been set.
			}else if(i->handle->revents != 0){

				// Set the last update tick.
				i->lastTick = currentTick;

				// Master socket has changed state, accept incoming sockets.
				if(i == scMasterDetails(sc)){

					socketHandle  clientHandle;
					socketDetails clientDetails;
					clientDetails.addressSize = sizeof(struct sockaddr);

					clientHandle.fd = accept(scMasterHandle(sc)->fd, (struct sockaddr *)&clientDetails.address, &clientDetails.addressSize);

					// Check if accept() was successful.
					if(clientHandle.fd != INVALID_SOCKET){
						clientHandle.events = POLLIN;
						clientHandle.revents = 0;
						clientDetails.lastTick = currentTick;
						clientDetails.flags = SOCKET_DETAILS_CONNECTED;
						scAddSocket(sc, &clientHandle, &clientDetails);
					}else{
						#ifdef SOCKET_DEBUG
						ssReportError("accept()", lastErrorID);
						#endif
						return 0;
					}

				// A client has changed state, receive incoming data.
				}else{

					// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in lastBuffer.
					i->lastBufferSize = recv(i->handle->fd, i->lastBuffer, SOCKET_MAX_BUFFER_SIZE, 0);

					if(i->lastBufferSize == -1){
						// Error encountered, disconnect problematic socket.
						socketclose(i->handle->fd);
						flagsSet(i->flags, SOCKET_DETAILS_ERROR);
						#ifdef SOCKET_DEBUG
						ssReportError("recv()", lastErrorID);
						#endif
						return 0;
					}else if(i->lastBufferSize == 0){
						// If the buffer is empty, the connection has closed.
						socketclose(i->handle->fd);
						flagsSet(i->flags, SOCKET_DETAILS_DISCONNECTED);
					}else{
						// Data received.
						flagsSet(i->flags, SOCKET_DETAILS_NEW_DATA);
					}

				}

				// Clear the revents flags.
				i->handle->revents = 0;
				--changedSockets;

			// Disconnect the current socket if it has timed out.
			}else if(flagsAreSet(flags, SOCKET_FLAGS_MANAGE_TIMEOUTS) && i != scMasterDetails(sc) && sdTimedOut(i, currentTick)){
				flagsSet(i->flags, SOCKET_DETAILS_TIMED_OUT);
			}

			--j;

		}

		++i;

	}

	return 1;

}

void ssShutdownTCP(socketConnectionHandler *const __RESTRICT__ sc){
	scDelete(sc);
}
