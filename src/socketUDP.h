#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

signed char ssSendDataUDP(const socketServer *server, const socketDetails *details, const char *msg);
signed char ssHandleConnectionsUDP(socketServer *server, const uint32_t currentTick, const unsigned char flags);
void ssShutdownUDP(socketServer *server);

#endif
