#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

unsigned char ssSendDataUDP(const socketServer *server, const socketDetails *details, const char *msg);
unsigned char ssDisconnectSocketUDP(socketServer *server, const size_t socketID);
unsigned char ssHandleConnectionsUDP(socketServer *server, const uint32_t currentTick, void (*ssHandleConnectUDP)(socketServer*, const socketHandle*, const socketDetails*), void (*ssHandleBufferUDP)(const socketServer*, const socketDetails*), void (*ssHandleDisconnectUDP)(socketServer*, const socketDetails*, const char), const unsigned char flags);
void ssShutdownUDP(socketServer *server);

#endif
