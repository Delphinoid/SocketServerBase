#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

return_t ssSendDataUDP(const socketServer *const __RESTRICT__ server, const socketDetails *const __RESTRICT__ details, const char *const __RESTRICT__ msg);
return_t ssHandleConnectionsUDP(socketServer *const __RESTRICT__ server, const uint32_t currentTick, const flags_t flags);
void ssShutdownUDP(socketServer *const __RESTRICT__ server);

#endif
