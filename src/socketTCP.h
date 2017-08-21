#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

void ssSendDataTCP(const socketHandle *clientHandle, const char *msg);
void ssDisconnectSocketTCP(socketServer *server, const size_t socketID);
void ssHandleConnectionsTCP(socketServer *server, const uint32_t currentTick, void (*ssHandleConnectTCP)(socketServer*, socketHandle*, socketDetails*), void (*ssHandleBufferTCP)(socketServer*, socketDetails), void (*ssHandleDisconnectTCP)(socketServer*, socketDetails, const char), const unsigned char flags);
void ssShutdownTCP(socketServer *server);

#endif
