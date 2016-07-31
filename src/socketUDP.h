#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketShared.h"

void ssSendDataUDP(socketServer *server, struct sockaddr_in *client, const char *msg);
void ssHandleBufferUDP(socketServer *server, struct sockaddr_in *sender);
void ssHandleConnectionsUDP(socketServer *server);

#endif
