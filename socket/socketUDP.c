#include "socketUDP.h"

#ifdef _WIN32
int ssSendUDP(const socketServer *const __RESTRICT__ server, const socketDetails *const __RESTRICT__ details, const char *const __RESTRICT__ buffer, const int length){
#else
int ssSendUDP(const socketServer *const __RESTRICT__ server, const socketDetails *const __RESTRICT__ details, const char *const __RESTRICT__ buffer, const size_t length){
#endif
	#ifdef SOCKET_DEBUG
	if(sendto(scMasterHandle(server)->fd, buffer, length, 0, (struct sockaddr *)&details->address, details->addressSize) < 0){
		ssReportError("sendto()", ssError);
		return 0;
	}
	return 1;
	#else
	return (sendto(scMasterHandle(server)->fd, buffer, length, 0, (struct sockaddr *)&details->address, details->addressSize) >= 0);
	#endif
}