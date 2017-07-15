#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

void ssSendDataUDP(socketServer *server, socketDetails details, const char *msg);
void ssHandleConnectionsUDP(socketServer *server, void (*ssHandleBufferUDP)(socketServer*, socketDetails), void (*ssHandleDisconnectUDP)(socketServer*));
void ssShutdownUDP(socketServer *server);

#endif
