#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include "socketServer.h"

return_t ssSendDataUDP(const socketConnectionHandler *const __RESTRICT__ connectionHandler, const socketDetails *const __RESTRICT__ details, const char *const __RESTRICT__ msg);
return_t ssHandleConnectionsUDP(socketConnectionHandler *const __RESTRICT__ connectionHandler, const uint32_t currentTick, const flags_t flags);
void ssShutdownUDP(socketConnectionHandler *const __RESTRICT__ connectionHandler);

#endif
