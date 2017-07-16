#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

void ssSendDataTCP(socketServer *server, int handle, const char *msg);
void ssDisconnectSocketTCP(socketServer *server, size_t socketID);
void ssHandleConnectionsTCP(socketServer *server, uint32_t currentTick, void (*ssHandleConnectTCP)(socketServer*, socketHandle*, socketDetails*), void (*ssHandleBufferTCP)(socketServer*, size_t), void (*ssHandleDisconnectTCP)(socketServer*, size_t, int), unsigned char flags);
void ssShutdownTCP(socketServer *server);

#endif
