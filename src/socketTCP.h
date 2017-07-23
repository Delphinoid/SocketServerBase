#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

void ssSendDataTCP(socketHandle *clientHandle, const char *msg);
void ssDisconnectSocketTCP(socketServer *server, size_t socketID);
void ssHandleConnectionsTCP(socketServer *server, uint32_t currentTick, void (*ssHandleConnectTCP)(socketServer*, socketHandle*, socketDetails*), void (*ssHandleBufferTCP)(socketServer*, socketDetails), void (*ssHandleDisconnectTCP)(socketServer*, socketDetails, char), unsigned char flags);
void ssShutdownTCP(socketServer *server);

#endif
