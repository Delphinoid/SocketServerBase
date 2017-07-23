#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

void ssSendDataUDP(socketServer *server, socketDetails details, const char *msg);
void ssDisconnectSocketUDP(socketServer *server, size_t socketID);
void ssHandleConnectionsUDP(socketServer *server, uint32_t currentTick, void (*ssHandleConnectUDP)(socketServer*, socketHandle*, socketDetails*), void (*ssHandleBufferUDP)(socketServer*, socketDetails), void (*ssHandleDisconnectUDP)(socketServer*, socketDetails, char), unsigned char flags);
void ssShutdownUDP(socketServer *server);

#endif
