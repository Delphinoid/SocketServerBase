#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

void ssSendDataTCP(socketServer *server, size_t socketID, const char *msg);
void ssDisconnectSocketTCP(socketServer *server, size_t socketID);
void ssHandleConnectionsTCP(socketServer *server, void (*ssHandleBufferTCP)(socketServer*, size_t), void (*ssHandleDisconnectTCP)(socketServer*, size_t));
void ssShutdownTCP(socketServer *server);

#endif
