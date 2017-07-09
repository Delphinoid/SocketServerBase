#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

void ssSendDataTCP(socketServer *server, size_t clientID, const char *msg);
void ssHandleConnectionsTCP(socketServer *server, void (*handleBuffer)(socketServer*, size_t), void (*handleDisconnect)(socketServer*, size_t));
void ssShutdownTCP(socketServer *server);

#endif
