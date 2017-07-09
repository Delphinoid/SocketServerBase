#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

size_t ssFindSocketUDP(socketServer *server, ssSocket *socket);
void   ssSendDataUDP(socketServer *server, ssSocket *socket, const char *msg);
void   ssHandleConnectionsUDP(socketServer *server, void (*handleBuffer)(socketServer*, ssSocket*), void (*handleDisconnect)(socketServer*));
void   ssShutdownUDP(socketServer *server);

#endif
