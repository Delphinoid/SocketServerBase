#include "socketShared.h"

#ifdef SOCK_USE_POLL
	#ifdef _WIN32
		WINSOCK_API_LINKAGE int WSAAPI WSAPoll(struct pollfd *ufds, ULONG fds, INT timeout);
		int pollFunc(socketHandle *ufds, size_t nfds, int timeout){ return WSAPoll(ufds, nfds, timeout); }
	#else
		int pollFunc(socketHandle *ufds, size_t nfds, int timeout){ return poll(ufds, nfds, timeout); }
	#endif
#else
	int pollFunc(socketHandle *ufds, size_t nfds, int timeout){
		int changedSockets;
		struct timeval timeoutConversion;
		struct timeval *timeoutVar = NULL;
		fd_set socketSet;
		size_t socketNum = nfds < SOCK_MAX_SOCKETS ? nfds : SOCK_MAX_SOCKETS;
		size_t i, j;
		for(i = 0; i < socketNum; ++i){
			socketSet.fd_array[i] = ufds[i].fd;
		}
		socketSet.fd_count = socketNum;
		if(timeout >= 0){
			if(timeout > 0){
				timeoutConversion.tv_sec = timeout / 1000;
				timeoutConversion.tv_usec = (timeout - (timeoutConversion.tv_sec * 1000)) * 1000;
			}else{
				timeoutConversion.tv_sec = 0;
				timeoutConversion.tv_usec = 0;
			}
			timeoutVar = &timeoutConversion;
		}
		changedSockets = select(0, &socketSet, NULL, NULL, timeoutVar);
		for(i = 0; i < changedSockets; ++i){
			for(j = 0; j < socketNum; j){
				if(socketSet.fd_array[i] == ufds[j].fd){
					ufds[j].revents = POLLIN;
					break;
				}
			}
		}
		return changedSockets;
	}
#endif
