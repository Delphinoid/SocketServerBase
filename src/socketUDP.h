#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

void ssSendDataUDP(const socketServer *server, const socketDetails details, const char *msg);
void ssDisconnectSocketUDP(socketServer *server, const size_t socketID);
void ssHandleConnectionsUDP(socketServer *server, const uint32_t currentTick, void (*ssHandleConnectUDP)(socketServer*, socketHandle*, socketDetails*), void (*ssHandleBufferUDP)(socketServer*, socketDetails), void (*ssHandleDisconnectUDP)(socketServer*, socketDetails, const char), const unsigned char flags);
void ssShutdownUDP(socketServer *server);

#endif
