#include "socketServer.h"
#include <string.h>
#include <stdio.h>

#ifndef _WIN32
	#include <fcntl.h>
#endif

void ssReportError(const char *const __RESTRICT__ failedFunction, const int errorCode){
	printf("\nSocket function %s has failed: %i\nSee here for more information:\nhttps://msdn.microsoft.com/en-us/library/windows/desktop/ms740668%%28v=vs.85%%29.aspx\n\n",
	       failedFunction, errorCode);
}

static inline return_t ssSetNonBlockMode(const int fd, unsigned long mode){
	#ifdef _WIN32
		return !ioctlsocket(fd, FIONBIO, &mode);
	#else
		int flags = fcntl(fd, F_GETFL, 0);
		if(flags < 0){
			return 0;
		}
		flags = mode ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
		return !fcntl(fd, F_SETFL, flags);
	#endif
}

static inline int ssGetAddressFamily(const char *const __RESTRICT__ ip){
	char buffer[16];
	if(inet_pton(AF_INET, ip, buffer)){
		return AF_INET;
	}else if(inet_pton(AF_INET6, ip, buffer)) {
		return AF_INET6;
	}
	return -1;
}

return_t ssInit(socketServer *const __RESTRICT__ server, const int type, const int protocol, const int argc, const char *argv[], return_t (*ssLoadConfig)(char(*)[45], uint16_t*, const int, const char**)){

	// Initialize server IP, port, type and protocol, then load the server config.
	struct pollfd masterHandle;
	socketDetails masterDetails;
	char ip[45]; ip[0] = '\0';
	int af;
	uint16_t port = SOCK_DEFAULT_PORT;

	puts("Initializing server...");

	server->type = type;
	server->protocol = protocol;
	if(ssLoadConfig != NULL){
		ssLoadConfig(&ip, &port, argc, argv);
	}

	// Create a socket prototype for the master socket.
	//
	// socket(address family, type, protocol)
	// address family = AF_UNSPEC, which can be either IPv4 or IPv6, AF_INET, which is IPv4 or AF_INET6, which is IPv6.
	// type = SOCK_STREAM, which uses TCP or SOCK_DGRAM, which uses UDP.
	// protocol = IPPROTO_TCP, which specifies to use TCP or IPPROTO_UDP, which specifies to use UDP.
	af = ssGetAddressFamily(ip);
	if(af == -1){
		af = SOCK_DEFAULT_ADDRESS_FAMILY;
	}
	masterHandle.fd = socket(af, server->type, server->protocol);
	if(masterHandle.fd == INVALID_SOCKET){  // If socket() failed, abort.
		ssReportError("socket()", lastErrorID);
		return 0;
	}
	masterHandle.events = POLLIN;
	masterHandle.revents = 0;

	// If SOCK_POLL_TIMEOUT isn't negative, we want a timeout for recfrom().
	if(SOCK_POLL_TIMEOUT > 0){
		// Set SO_RCVTIMEO
		const unsigned long timeout = SOCK_POLL_TIMEOUT;
		if(setsockopt(masterHandle.fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR){
			ssReportError("setsockopt()", lastErrorID);
			return 0;
		}
	}else if(SOCK_POLL_TIMEOUT == 0){
		ssSetNonBlockMode(masterHandle.fd, 1);
	}

	// Bind the master socket to the host address.
	if(af == AF_INET){

		// Create the sockaddr_in structure using the master socket's address family and the supplied IP / port.
		struct sockaddr_in serverAddress4;
		if(!inet_pton(af, ip, (char*)&(serverAddress4.sin_addr))){
			serverAddress4.sin_addr.s_addr = INADDR_ANY;
			strcpy(ip, "INADDR_ANY");
		}
		serverAddress4.sin_family = af;
		serverAddress4.sin_port = htons(port);  // htons() converts the port from big-endian to little-endian for sockaddr_in.
		masterDetails.address = *((struct sockaddr_storage *)&serverAddress4);

	}else if(af == AF_INET6){  // Address is IPv6

		// Create the sockaddr_in6 structure using the master socket's address family and the supplied IP / port.
		struct sockaddr_in6 serverAddress6;
		memset(&serverAddress6, 0, sizeof(struct sockaddr_in6));  // Set everything in sockaddr_in6 to 0, as there are a number of fields we don't otherwise set.
		if(!inet_pton(af, ip, (char*)&(serverAddress6.sin6_addr))){
			serverAddress6.sin6_addr = in6addr_any;
			strcpy(ip, "in6addr_any");
		}
		serverAddress6.sin6_family = af;
		serverAddress6.sin6_port = htons(port);  // htons() converts the port from big-endian to little-endian for sockaddr_in6.
		masterDetails.address = *((struct sockaddr_storage *)&serverAddress6);

	}

	// Check result of bind()
	if(bind(masterHandle.fd, (struct sockaddr *)&masterDetails.address, sizeof(masterDetails.address)) == SOCKET_ERROR){  // If bind() failed, abort.
		ssReportError("bind()", lastErrorID);
		return 0;
	}

	// If the server is operating over TCP, set the master socket's state to "listen" so it will start listening for incoming connections from sockets.
	if(server->protocol == IPPROTO_TCP){
		if(listen(masterHandle.fd, SOMAXCONN) == SOCKET_ERROR){  // SOMAXCONN = automatically choose maximum number of pending connections, different across systems.
			ssReportError("listen()", lastErrorID);
			return 0;
		}
	}

	// Initialize the connection handler.
	if(!scdInit(&server->connectionHandler, SOCK_MAX_SOCKETS, &masterHandle, &masterDetails)){
		puts("Error: the socket connection handler could not be initialized.\n");
		return 0;
	}

	printf("Socket successfully initialized on %s:%u.\n\n", ip, port);
	return 1;

}

inline socketHandle *ssGetSocketHandle(const socketServer *const __RESTRICT__ server, const size_t socketID){
	return &server->connectionHandler.handles[server->connectionHandler.idLinks[socketID]];
}

inline socketDetails *ssGetSocketDetails(const socketServer *const __RESTRICT__ server, const size_t socketID){
	return &server->connectionHandler.details[server->connectionHandler.idLinks[socketID]];
}

return_t ssSocketTimedOut(socketServer *const __RESTRICT__ server, const size_t socketID, const uint32_t currentTick){
	if(socketID > 0){
		return currentTick - ssGetSocketDetails(server, socketID)->lastUpdateTick >= SOCK_CONNECTION_TIMEOUT;
	}
	return 0;
}

void ssCheckTimeouts(socketServer *const __RESTRICT__ server, const uint32_t currentTick){
	// This function is slow and mostly unnecessary, so it should be avoided if at all possible!
	size_t i;
	for(i = 1; i < server->connectionHandler.size; ++i){
		// Disconnect the socket at index i if it has timed out.
		if(ssSocketTimedOut(server, server->connectionHandler.details[i].id, currentTick)){
			// UDP sockets use the same handle as the master socket, so they won't be closed.
			if(server->connectionHandler.handles[i].fd != server->connectionHandler.handles[0].fd){
				socketclose(server->connectionHandler.handles[i].fd);
			}
			scdRemoveSocket(&server->connectionHandler, server->connectionHandler.details[i].id);
			--i;
		}
	}
}

#ifdef _WIN32
	return_t ssStartup(){
		// Initialize Winsock.
		WSADATA wsaData;
		int initError = WSAStartup(WINSOCK_VERSION, &wsaData);

		if(initError != 0){  // If Winsock did not initialize correctly, abort.
			ssReportError("WSAStartup()", initError);
			return 0;
		}
		return 1;
	}

	void ssCleanup(){
		WSACleanup();
	}
#endif