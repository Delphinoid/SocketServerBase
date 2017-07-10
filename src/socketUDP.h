#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

size_t ssFindSocketUDP(socketServer *server, ssSocket *socket);
void   ssSendDataUDP(socketServer *server, ssSocket *socket, const char *msg);
void   ssHandleConnectionsUDP(socketServer *server, void (*ssHandleBufferUDP)(socketServer*, ssSocket*), void (*ssHandleDisconnectUDP)(socketServer*));
void   ssShutdownUDP(socketServer *server);

#endif
