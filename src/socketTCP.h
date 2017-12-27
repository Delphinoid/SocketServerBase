#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

unsigned char ssSendDataTCP(const socketHandle *clientHandle, const char *msg);
unsigned char ssDisconnectSocketTCP(socketServer *server, const size_t socketID);
unsigned char ssHandleConnectionsTCP(socketServer *server, const uint32_t currentTick, void (*ssHandleConnectTCP)(socketServer*, const socketHandle*, const socketDetails*), void (*ssHandleBufferTCP)(const socketServer*, const socketDetails*), void (*ssHandleDisconnectTCP)(socketServer*, const socketDetails*, const char), const unsigned char flags);
void ssShutdownTCP(socketServer *server);

#endif
