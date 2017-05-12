#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

void ssSendDataTCP(socketServer *server, unsigned int clientID, const char *msg);
void ssHandleConnectionsTCP(socketServer *server, void (*handleBuffer)(socketServer*, unsigned int), void (*handleDisconnect)(socketServer*, unsigned int));
void ssShutdownTCP(socketServer *server);

#endif
