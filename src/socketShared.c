#include "socketShared.h"
#include <string.h>
#include <stdio.h>

int inet_pton(int af, const char *src, char *dst);

void ssReportError(const char *failedFunction, int errorCode){

	printf("Socket function %s has failed: %i\nSee here for more information:\nhttps://msdn.microsoft.com/en-us/library/windows/desktop/ms740668%%28v=vs.85%%29.aspx\n",
	       failedFunction, errorCode);

}

void ssSubstringHelper(char *strTarget, char *strCopy, unsigned int pos, unsigned int length){

	if(pos + length <= strlen(strCopy)){
		unsigned int d;
		for(d = 0; d < length; d++){
			if(strCopy[pos + d] == '\n'){
				d = length;
			}else{
				strTarget[d] = strCopy[pos + d];
			}
		}
		strTarget[length] = '\0';
	}

}

int ssLoadConfig(socketServer *server, const char *prgPath){

	char *cfgPath = (char*)prgPath;
	cfgPath[strrchr(cfgPath, '\\') - cfgPath + 1] = '\0';  // Removes program name (everything after the last backslash) from the path
	strcpy(cfgPath + strlen(cfgPath), "config.txt");  // Append "config.txt" to the end

	FILE *serverConfig = fopen(cfgPath, "r");
	char line[1000];
	char compare[8];
	char lineData[1000];
	char *strtolPtr;

	if(serverConfig != NULL){
		while(!feof(serverConfig)){

			fgets(line, sizeof(line), serverConfig);

			// Remove any comments from the line
			char *commentPos = strstr(line, "//");
			if(commentPos != NULL){
				commentPos = '\0';
			}

			if(strlen(line) > 7){
				ssSubstringHelper(compare, line, 0, 5);
				if(strcmp(compare, "ip = ") == 0){
					ssSubstringHelper(lineData, line, 5, strlen(line) - 6);
					if(strlen(lineData) <= 39){
						strcpy(server->ip, lineData);
					}
				}
				ssSubstringHelper(compare, line, 0, 7);
				if(strcmp(compare, "port = ") == 0){
					ssSubstringHelper(lineData, line, 7, strlen(line) - 7);
					server->port = strtol(lineData, &strtolPtr, 10);
				}

			}

		}

		fclose(serverConfig);
		printf("Config loaded.\n");
		return 1;

	}else{
		printf("Error loading config file:\nThe specified config path is invalid. ");
	}

	fclose(serverConfig);
	printf("No config loaded.\n");
	return 0;

}

int ssInit(socketServer *server, unsigned int type, const int argc, const char *argv[]){

	/* Initialize server IP and port, then load the server config */
	server->ip[0] = '\0';
	server->port = DEFAULT_PORT;

	if(argc > 0){  /** NEEDS FIXING **/
		ssLoadConfig(server, argv[0]);
	}


	/* Specify the version of Winsock to use and initialize it */
	#ifdef _WIN32
		WSADATA wsaData;
		int initError = WSAStartup(WINSOCK_VERSION, &wsaData);

		if(initError != 0){  // If Winsock did not initialize correctly, abort
			ssReportError("WSAStartup()", initError);
			return 0;
		}
	#endif


	/* Create a socket prototype for the master socket */
	/*
	   socket(address family, type, protocol)
	   address family = AF_UNSPEC, which can be either IPv4 or IPv6
	   type = SOCK_STREAM, which uses TCP or SOCK_DGRAM, which uses UDP
	   protocol = IPPROTO_TCP, which specifies to use TCP or IPPROTO_TCP, which specifies to use UDP
	*/
	unsigned int stageFailed = 1;
	if(type == INIT_TCP){  // Only create a prototype for the TCP socket
		server->masterSocketTCP = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);  // TCP socket
		stageFailed = server->masterSocketTCP == INVALID_SOCKET;
	}else if(type == INIT_UDP){  // Only create a prototype for the UDP socket
		server->masterSocketUDP = socket(AF_UNSPEC, SOCK_DGRAM,  IPPROTO_UDP);  // UDP socket
		stageFailed = server->masterSocketUDP == INVALID_SOCKET;
	}else if(type == INIT_TCP_AND_UDP){  // Create prototypes for both sockets
		server->masterSocketTCP = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);  // TCP socket
		server->masterSocketUDP = socket(AF_UNSPEC, SOCK_DGRAM,  IPPROTO_UDP);  // UDP socket
		stageFailed = server->masterSocketTCP == INVALID_SOCKET ||
		              server->masterSocketUDP == INVALID_SOCKET;
	}else{
		printf("Error initializing socket:\nThe specified communication protocol is invalid.\n");
		return 0;
	}

	if(stageFailed){  // If socket() failed, abort
		ssReportError("socket()", lastErrorID);
		#ifdef _WIN32
			WSACleanup();
		#endif
		return 0;
	}


	/* Bind the master socket to the host address */
	WSAPROTOCOL_INFO protocolInfo;
	// Retrieve details about the master socket (we want the address family being used)
	if(type == INIT_TCP){
		WSADuplicateSocket(server->masterSocketTCP, GetCurrentProcessId(), &protocolInfo);
	}else{
		WSADuplicateSocket(server->masterSocketUDP, GetCurrentProcessId(), &protocolInfo);
	}

	// Create the sockaddr_in structure using the master socket's address family and the supplied IP / port
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = protocolInfo.iAddressFamily;

	if(strlen(server->ip) > 0){  // If the IP has been specified, convert it from a string to the in_addr format for sockaddr_in
		inet_pton(protocolInfo.iAddressFamily, server->ip, (char*)&(serverAddress.sin_addr));
	}else{	  // Otherwise use all available addresses
		serverAddress.sin_addr.s_addr = INADDR_ANY;
	}

	serverAddress.sin_port = htons(server->port);  // htons() converts the port from big-endian to little-endian for sockaddr_in

	if(type == INIT_TCP){  // Only bind the address to the TCP socket
		stageFailed = bind(server->masterSocketTCP, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR;
	}else if(type == INIT_UDP){  // Only bind the address to the UDP socket
		stageFailed = bind(server->masterSocketUDP, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR;
	}else if(type == INIT_TCP_AND_UDP){  // Bind the address to both sockets
		stageFailed = bind(server->masterSocketTCP, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR ||
		              bind(server->masterSocketUDP, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR;
	}

	if(stageFailed){  // If bind() failed, abort
		ssReportError("bind()", WSAGetLastError());
		#ifdef _WIN32
			WSACleanup();
		#endif
		return 0;
	}


	/* Change the TCP master socket's state to "listen" so it will start listening for incoming connections from sockets */
	if(type == INIT_TCP || type == INIT_TCP_AND_UDP){
		if(listen(server->masterSocketTCP, SOMAXCONN) == SOCKET_ERROR){  // SOMAXCONN = automatically choose maximum number of pending connections, different across systems
			ssReportError("listen()", lastErrorID);
			#ifdef _WIN32
				WSACleanup();
			#endif
			return 0;
		}
	}


	/* Initialize connectedSockets vector */
	svInit(&server->connectedSockets, 1);


	printf("Server is up!\n\n");
	return 1;

}

int ssSocketHasConnected(socketServer *server, struct sockaddr_in *socketDetails){

	return 1;  // Can't be bothered doing this for now. It's mostly implementation-dependent anyway

}

void ssDisconnectSocket(socketServer *server, unsigned int socketID){
	closesocket(svGet(&server->connectedSockets, socketID));
	FD_CLR(svGet(&server->connectedSockets, socketID), &server->socketSet);
	svErase(&server->connectedSockets, server->connectedSockets.buffer[0] + socketID);
}

void ssShutdown(socketServer *server){

	unsigned int d;
	for(d = 0; d < server->connectedSockets.size; d++){
		closesocket(svGet(&server->connectedSockets, d));
	}
	svClear(&server->connectedSockets);
	FD_ZERO(&server->socketSet);

	#ifdef _WIN32
		WSACleanup();
	#endif

}
