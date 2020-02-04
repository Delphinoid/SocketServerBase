#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include "socketServer.h"

return_t ssSendDataTCP(const socketHandle *const __RESTRICT__ clientHandle, const char *const __RESTRICT__ msg);
return_t ssHandleConnectionsTCP(socketServer *const __RESTRICT__ server, const uint32_t currentTick, const flags_t flags);
void ssShutdownTCP(socketServer *const __RESTRICT__ server);

#endif
