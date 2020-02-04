#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

return_t ssSendDataTCP(const socketHandle *const __RESTRICT__ clientHandle, const char *const __RESTRICT__ msg);
return_t ssDisconnectSocketTCP(socketConnectionHandler *const __RESTRICT__ connectionHandler, socketDetails *clientDetails);
return_t ssHandleConnectionsTCP(socketConnectionHandler *const __RESTRICT__ connectionHandler, const uint32_t currentTick, const flags_t flags);
void ssShutdownTCP(socketConnectionHandler *const __RESTRICT__ connectionHandler);

#endif
