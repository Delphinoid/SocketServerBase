#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

void ssSendDataUDP(socketServer *server, struct sockaddr_storage *client, const char *msg);
void ssHandleConnectionsUDP(socketServer *server, void (*handleBuffer)(socketServer*, struct sockaddr_storage*), void (*handleDisconnect)(socketServer*));
void ssShutdownUDP(socketServer *server);

#endif
