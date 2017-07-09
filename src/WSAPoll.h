#ifndef WSAPOLL_H
#define WSAPOLL_H

#define POLLIN   0x0001
#define POLLPRI  0x0002
#define POLLOUT  0x0004
#define POLLERR  0x0008
#define POLLHUP  0x0010
#define POLLNVAL 0x0020

typedef struct pollfd {
	SOCKET fd;
	short events;
	short revents;
} WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD, pollfd;

/*WINSOCK_API_LINKAGE int WSAAPI WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout);
inline int poll(struct pollfd *pfd, int nfds, int timeout){ return WSAPoll(pfd, nfds, timeout); }*/

#endif
