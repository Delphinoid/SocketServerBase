#include "socketServer.h"
#include <stdio.h>

int ssGetAddressFamily(const char *ip){
	char buffer[16];
	if(inet_pton(AF_INET, ip, buffer)){
		return AF_INET;
	}else if(inet_pton(AF_INET6, ip, buffer)) {
		return AF_INET6;
	}
	return -1;
}

void ssReportError(const char *failedFunction, int errorCode){
	printf("\nSocket function %s has failed: %i\nSee here for more information:\nhttps://msdn.microsoft.com/en-us/library/windows/desktop/ms740668%%28v=vs.85%%29.aspx\n\n",
	       failedFunction, errorCode);
}

int ssInit(socketServer *server, int type, int protocol, const int argc, const char *argv[], int (*loadConfig)(socketServer*, const int, const char**)){

	printf("Initializing socket...\n");

	/* Initialize server IP, port, type and protocol, then load the server config */
	server->ip[0] = '\0';
	server->port = DEFAULT_PORT;
	server->type = type;
	server->protocol = protocol;
	if(loadConfig != NULL){
		loadConfig(server, argc, argv);
	}

	/* Create a socket prototype for the master socket */
	/*
	   socket(address family, type, protocol)
	   address family = AF_UNSPEC, which can be either IPv4 or IPv6, AF_INET, which is IPv4 or AF_INET6, which is IPv6
	   type = SOCK_STREAM, which uses TCP or SOCK_DGRAM, which uses UDP
	   protocol = IPPROTO_TCP, which specifies to use TCP or IPPROTO_UDP, which specifies to use UDP
	*/
	server->af = ssGetAddressFamily(server->ip);
	if(server->af == -1){
		server->af = DEFAULT_ADDRESS_FAMILY;
	}
	server->masterSocket = socket(server->af, server->type, server->protocol);
	if(server->masterSocket == INVALID_SOCKET){  // If socket() failed, abort
		ssReportError("socket()", lastErrorID);
		return 0;
	}

	/* Bind the master socket to the host address */
	int bindStatus = SOCKET_ERROR;

	if(server->af == AF_INET){

		// Create the sockaddr_in structure using the master socket's address family and the supplied IP / port
		struct sockaddr_in serverAddress4;
		if(!inet_pton(server->af, server->ip, (char*)&(serverAddress4.sin_addr))){
			serverAddress4.sin_addr.s_addr = INADDR_ANY;
			strcpy(server->ip, "INADDR_ANY");
		}
		serverAddress4.sin_family = server->af;
		serverAddress4.sin_port = htons(server->port);  // htons() converts the port from big-endian to little-endian for sockaddr_in
		// Bind the address to the socket
		bindStatus = bind(server->masterSocket, (struct sockaddr*)&serverAddress4, sizeof(serverAddress4));

	}else if(server->af == AF_INET6){  // Address is IPv6

		// Create the sockaddr_in6 structure using the master socket's address family and the supplied IP / port
		struct sockaddr_in6 serverAddress6;
		memset(&serverAddress6, 0, sizeof(struct sockaddr_in6));  // Set everything in sockaddr_in6 to 0, as there are a number of fields we don't otherwise set
		if(!inet_pton(server->af, server->ip, (char*)&(serverAddress6.sin6_addr))){
			serverAddress6.sin6_addr = in6addr_any;
			strcpy(server->ip, "in6addr_any");
		}
		serverAddress6.sin6_family = server->af;
		serverAddress6.sin6_port = htons(server->port);  // htons() converts the port from big-endian to little-endian for sockaddr_in6
		// Bind the address to the socket
		bindStatus = bind(server->masterSocket, (struct sockaddr*)&serverAddress6, sizeof(serverAddress6));

	}

	// Check result of bind()
	if(bindStatus == SOCKET_ERROR){  // If bind() failed, abort
		ssReportError("bind()", lastErrorID);
		return 0;
	}

	/* If the server is operating over TCP, set the master socket's state to "listen" so it will start listening for incoming connections from sockets */
	if(server->protocol == IPPROTO_TCP){
		if(listen(server->masterSocket, SOMAXCONN) == SOCKET_ERROR){  // SOMAXCONN = automatically choose maximum number of pending connections, different across systems
			ssReportError("listen()", lastErrorID);
			return 0;
		}
	}

	/* Initialize the connectedSockets vector */
	cvInit(&server->connectedSockets, 1);

	printf("Socket successfully initialized on %s:%u.\n\n", server->ip, server->port);
	return 1;

}

#ifdef _WIN32
	int ssStartup(){
		/* Initialize Winsock */
		WSADATA wsaData;
		int initError = WSAStartup(WINSOCK_VERSION, &wsaData);

		if(initError != 0){  // If Winsock did not initialize correctly, abort
			ssReportError("WSAStartup()", initError);
			return 0;
		}
		return 1;
	}

	void ssCleanup(){
		WSACleanup();
	}
#endif
