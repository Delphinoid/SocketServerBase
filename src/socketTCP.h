#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

signed char ssSendDataTCP(const socketHandle *clientHandle, const char *msg);
signed char ssHandleConnectionsTCP(socketServer *server, const uint32_t currentTick, const unsigned char flags);
void ssShutdownTCP(socketServer *server);

#endif
