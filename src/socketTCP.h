#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketShared.h"

void ssSendDataTCP(socketServer *server, unsigned int clientID, const char *msg);
void ssHandleBufferTCP(socketServer *server, unsigned int senderID);
void ssHandleConnectionsTCP(socketServer *server);

#endif
