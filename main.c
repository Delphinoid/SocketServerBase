#define WIN32_LEAN_AND_MEAN
#ifndef SOCKET_USE_MALLOC
#include "memory/memoryManager.h"
#endif
#include "socket/socketTCP.h"
#include "socket/socketUDP.h"
#include <string.h>
#include <stdio.h>

socketServer testServerTCP;
socketServer testServerUDP;

return_t ssConfigLoad(ssConfig *const config, char **argv){

	char *cfgPath = argv[0];
	memcpy(strrchr(cfgPath, '\\') + 1, "config\0", 7);  // Append "config" to the end.

	FILE *serverConfig = fopen(cfgPath, "r");
	char lineFeed[1024];
	char *line;
	size_t lineLength;

	if(serverConfig != NULL){
		while(fgets(lineFeed, sizeof(lineFeed), serverConfig)){

			line = lineFeed;
			lineLength = strlen(line);

			// Remove new line and carriage return.
			if(line[lineLength-1] == '\n'){
				line[--lineLength] = '\0';
			}
			if(line[lineLength-1] == '\r'){
				line[--lineLength] = '\0';
			}
			// Remove any comments from the line.
			char *commentPos = strstr(line, "//");
			if(commentPos != NULL){
				lineLength -= commentPos-line;
				*commentPos = '\0';
			}
			// Remove any indentations from the line, as well as any trailing spaces and tabs.
			unsigned char doneFront = 0, doneEnd = 0;
			size_t newOffset = 0;
			size_t i;
			for(i = 0; (i < lineLength && !doneFront && !doneEnd); ++i){
				if(!doneFront && line[i] != '\t' && line[i] != ' '){
					newOffset = i;
					doneFront = 1;
				}
				if(!doneEnd && i > 1 && i < lineLength && line[lineLength-i] != '\t' && line[lineLength-i] != ' '){
					lineLength -= i-1;
					line[lineLength] = '\0';
					doneEnd = 1;
				}
			}
			line += newOffset;
			lineLength -= newOffset;

			if(lineLength > 7){
				// IP
				if(strncmp(line, "ip = ", 5) == 0){
					strncpy(config->ip, line+5, 40);

				// Port
				}else if(strncmp(line, "port = ", 7) == 0){
					config->port = strtol(line+7, NULL, 0);

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

void ssHandleConnectTCP(socketServer *server, socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Accepted TCP connection from %s (socket #%d).\n", IP, details->handle->fd);
}

void ssHandleBufferTCP(const socketServer *server, socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Data received over TCP from %s (socket #%d): %s\n", IP, details->handle->fd, details->lastBuffer);
	ssSendDataTCP(details->handle, "Data received over TCP successfully. You should get this.\n");
}

void ssHandleDisconnectTCP(socketServer *server, socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Closing TCP connection with %s (socket #%d).\n", IP, details->handle->fd);
	scRemoveSocket(&server->connectionHandler, details);
}

void ssHandleConnectUDP(socketServer *server, socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Accepted UDP connection from %s (socket #%d).\n", IP, details->handle->fd);
}

void ssHandleBufferUDP(const socketServer *server, socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Data received over UDP from %s: %s\n", IP, details->lastBuffer);
	ssSendDataUDP(&server->connectionHandler, details, "Data received over UDP successfully. You might get this.\n");
}

void ssHandleDisconnectUDP(socketServer *server, socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Closing UDP connection with %s (socket #%d).\n", IP, details->handle->fd);
	scRemoveSocket(&server->connectionHandler, details);
}

void cleanup(){
	ssShutdownTCP(&testServerTCP.connectionHandler);
	ssShutdownUDP(&testServerUDP.connectionHandler);
	#ifdef _WIN32
	ssCleanup();
	#endif
	#ifndef SOCKET_USE_MALLOC
	memMngrDelete();
	#endif
}

int main(int argc, char **argv){

	unsigned char flagsUDP = SOCKET_FLAGS_ABSTRACT_HANDLE | SOCKET_FLAGS_READ_FULL_QUEUE;
	unsigned char flagsTCP = 0x00;

	ssConfig configTCP = {
		.type = SOCK_STREAM,
		.protocol = IPPROTO_TCP,
		.backlog = SOMAXCONN,
		.connections = SOCKET_MAX_SOCKETS
	};
	ssConfig configUDP = {
		.type = SOCK_DGRAM,
		.protocol = IPPROTO_UDP,
		.backlog = SOMAXCONN,
		.connections = SOCKET_MAX_SOCKETS
	};
	ssConfigLoad(&configTCP, argv);
	ssConfigLoad(&configUDP, argv);
	puts("");

	if(
		#ifndef SOCKET_USE_MALLOC
		memMngrInit(MEMORY_MANAGER_DEFAULT_VIRTUAL_HEAP_SIZE, 1) < 0 ||
		#endif
		!ssStartup() ||
		!ssInit(&testServerTCP, configTCP) ||
		!ssInit(&testServerUDP, configUDP)
	){
		return 1;
	}

	atexit(cleanup);

	while(1){

		socketDetails *i;
		size_t j;

		if(
			!ssHandleConnectionsUDP(&testServerUDP.connectionHandler, 0, flagsUDP) ||
			!ssHandleConnectionsTCP(&testServerTCP.connectionHandler, 0, flagsTCP)
		){
			break;
		}

		i = testServerTCP.connectionHandler.details;
		j = testServerTCP.connectionHandler.nfds;
		while(j > 0){
			if(sdValid(i)){
				if(flagsAreSet(i->flags, SOCKET_DETAILS_CONNECTED)){
					// Socket has connected.
					ssHandleConnectTCP(&testServerTCP, i);
					flagsUnset(i->flags, SOCKET_DETAILS_CONNECTED);
				}
				if(flagsAreSet(i->flags, SOCKET_DETAILS_NEW_DATA)){
					// Socket has sent data.
					ssHandleBufferTCP(&testServerTCP, i);
					flagsUnset(i->flags, SOCKET_DETAILS_NEW_DATA);
				}
				if(
					flagsAreSet(i->flags, SOCKET_DETAILS_DISCONNECTED) ||
					flagsAreSet(i->flags, SOCKET_DETAILS_TIMED_OUT) ||
					flagsAreSet(i->flags, SOCKET_DETAILS_ERROR)
				){
					// Socket has disconnected.
					ssHandleDisconnectTCP(&testServerTCP, i);
					flagsUnset(i->flags, SOCKET_DETAILS_DISCONNECTED | SOCKET_DETAILS_TIMED_OUT | SOCKET_DETAILS_ERROR);
				}
				--j;
			}
			++i;
		}

		i = testServerUDP.connectionHandler.details;
		j = testServerUDP.connectionHandler.nfds;
		while(j > 0){
			if(sdValid(i)){
				if(flagsAreSet(i->flags, SOCKET_DETAILS_CONNECTED)){
					// Socket has connected.
					ssHandleConnectUDP(&testServerUDP, i);
					flagsUnset(i->flags, SOCKET_DETAILS_CONNECTED);
				}
				if(flagsAreSet(i->flags, SOCKET_DETAILS_NEW_DATA)){
					// Socket has sent data.
					ssHandleBufferUDP(&testServerUDP, i);
					flagsUnset(i->flags, SOCKET_DETAILS_NEW_DATA);
				}
				if(
					flagsAreSet(i->flags, SOCKET_DETAILS_DISCONNECTED) ||
					flagsAreSet(i->flags, SOCKET_DETAILS_TIMED_OUT) ||
					flagsAreSet(i->flags, SOCKET_DETAILS_ERROR)
				){
					// Socket has disconnected.
					ssHandleDisconnectUDP(&testServerUDP, i);
					flagsUnset(i->flags, SOCKET_DETAILS_DISCONNECTED | SOCKET_DETAILS_TIMED_OUT | SOCKET_DETAILS_ERROR);
				}
				--j;
			}
			++i;
		}

	}

	return 0;

}
