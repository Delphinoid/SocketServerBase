#include "socketShared.h"
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

int ssInitShared(socketShared *hostData, int hostType, int hostProtocol){

	/* Create a socket prototype for the master socket */
	/*
	   socket(address family, type, protocol)
	   address family = AF_UNSPEC, which can be either IPv4 or IPv6, AF_INET, which is IPv4 or AF_INET6, which is IPv6
	   type = SOCK_STREAM, which uses TCP or SOCK_DGRAM, which uses UDP
	   protocol = IPPROTO_TCP, which specifies to use TCP or IPPROTO_TCP, which specifies to use UDP
	*/
	hostData->masterSocket = socket(hostData->af, hostType, hostProtocol);
	if(hostData->masterSocket == INVALID_SOCKET){  // If socket() failed, abort
		ssReportError("socket()", lastErrorID);
		return 0;
	}


	/* Bind the master socket to the host address */
	int bindStatus = SOCKET_ERROR;

	if(hostData->af == AF_INET){

		// Create the sockaddr_in structure using the master socket's address family and the supplied IP / port
		struct sockaddr_in serverAddress4;
		if(!inet_pton(hostData->af, hostData->ip, (char*)&(serverAddress4.sin_addr))){
			serverAddress4.sin_addr.s_addr = INADDR_ANY;
			strcpy(hostData->ip, "INADDR_ANY");
		}
		serverAddress4.sin_family = hostData->af;
		serverAddress4.sin_port = htons(hostData->port);  // htons() converts the port from big-endian to little-endian for sockaddr_in
		// Bind the address to the socket
		bindStatus = bind(hostData->masterSocket, (struct sockaddr*)&serverAddress4, sizeof(serverAddress4));

	}else if(hostData->af == AF_INET6){  // Address is IPv6

		// Create the sockaddr_in6 structure using the master socket's address family and the supplied IP / port
		struct sockaddr_in6 serverAddress6;
		memset(&serverAddress6, 0, sizeof(struct sockaddr_in6));  // Set everything in sockaddr_in6 to 0, as there are a number of fields we don't otherwise set
		if(!inet_pton(hostData->af, hostData->ip, (char*)&(serverAddress6.sin6_addr))){
			serverAddress6.sin6_addr = in6addr_any;
			strcpy(hostData->ip, "in6addr_any");
		}
		serverAddress6.sin6_family = hostData->af;
		serverAddress6.sin6_port = htons(hostData->port);  // htons() converts the port from big-endian to little-endian for sockaddr_in6
		// Bind the address to the socket
		bindStatus = bind(hostData->masterSocket, (struct sockaddr*)&serverAddress6, sizeof(serverAddress6));

	}

	// Check result of bind()
	if(bindStatus == SOCKET_ERROR){  // If bind() failed, abort
		ssReportError("bind()", lastErrorID);
		return 0;
	}


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
