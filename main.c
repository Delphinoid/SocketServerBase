#define WIN32_LEAN_AND_MEAN
#ifndef SOCKET_USE_MALLOC
#include "memory/memoryManager.h"
#endif
#include "socket/socketTCP.h"
#include "socket/socketUDP.h"
#include "shared/flags.h"
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

void ssHandleConnectTCP(const socketServer *const __RESTRICT__ server, const socketDetails *const details){
	char ip[46];
	inet_ntop(
		details->address.ss_family,
		(details->address.ss_family == AF_INET ?
		(void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
		(void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
		ip, sizeof(ip)
	);
	printf("Accepted TCP connection from %s:%u (socket #%lu).\n", ip, ((struct sockaddr_in *)&details->address)->sin_port, (unsigned long)details->id);
}

void ssHandleBufferTCP(const socketServer *const __RESTRICT__ server, const socketDetails *const details, char *const __RESTRICT__ buffer, const int bufferSize){
	char ip[46];
	inet_ntop(
		details->address.ss_family,
		(details->address.ss_family == AF_INET ?
		(void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
		(void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
		ip, sizeof(ip)
	);
	buffer[bufferSize] = '\0';
	printf("Data received over TCP from %s:%u (socket #%lu): %s\n", ip, ((struct sockaddr_in *)&details->address)->sin_port, (unsigned long)details->id, buffer);
	ssSendTCP(details->handle, "Data received over TCP successfully. You should get this.\n", 58);
}

void ssHandleDisconnectTCP(socketServer *const __RESTRICT__ server, socketDetails *const details){
	char ip[46];
	inet_ntop(
		details->address.ss_family,
		(details->address.ss_family == AF_INET ?
		(void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
		(void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
		ip, sizeof(ip)
	);
	printf("Closing TCP connection with %s:%u (socket #%lu).\n", ip, ((struct sockaddr_in *)&details->address)->sin_port, (unsigned long)details->id);
	ssDisconnect(server, details);
}

void ssHandleConnectUDP(const socketServer *const __RESTRICT__ server, const socketDetails *const details){
	char ip[46];
	inet_ntop(
		details->address.ss_family,
		(details->address.ss_family == AF_INET ?
		(void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
		(void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
		ip, sizeof(ip)
	);
	printf("Accepted UDP connection from %s:%u (socket #%lu).\n", ip, ((struct sockaddr_in *)&details->address)->sin_port, (unsigned long)details->id);
}

void ssHandleBufferUDP(const socketServer *const __RESTRICT__ server, const socketDetails *const __RESTRICT__ details, char *const __RESTRICT__ buffer, const int bufferSize){
	char ip[46];
	inet_ntop(
		details->address.ss_family,
		(details->address.ss_family == AF_INET ?
		(void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
		(void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
		ip, sizeof(ip)
	);
	buffer[bufferSize] = '\0';
	printf("Data received over UDP from %s:%u (socket #%lu): %s\n", ip, ((struct sockaddr_in *)&details->address)->sin_port, (unsigned long)details->id, buffer);
	ssSendUDP(server, details, "Data received over UDP successfully. You might get this.\n", 57);
}

void ssHandleDisconnectUDP(socketServer *server, socketDetails *details){
	char ip[46];
	inet_ntop(
		details->address.ss_family,
		(details->address.ss_family == AF_INET ?
		(void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
		(void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
		ip, sizeof(ip)
	);
	printf("Closing UDP connection with %s:%u (socket #%lu).\n", ip, ((struct sockaddr_in *)&details->address)->sin_port, (unsigned long)details->id);
	scRemoveSocket(server, details);
}

void cleanup(){
	ssDelete(&testServerUDP);
	ssDelete(&testServerTCP);
	#ifdef _WIN32
	ssCleanup();
	#endif
	#ifndef SOCKET_USE_MALLOC
	memMngrDelete();
	#endif
}

int main(int argc, char **argv){

	char buffer[SOCKET_MAX_BUFFER_SIZE];
	int bufferSize;

	ssConfig configTCP = {
		.type = SOCK_STREAM,
		.protocol = IPPROTO_TCP,
		.allocate = SOCKET_SERVER_ALLOCATE_EVERYTHING,
		.backlog = SOMAXCONN,
		.connections = SOCKET_MAX_SOCKETS
	};
	ssConfig configUDP = {
		.type = SOCK_DGRAM,
		.protocol = IPPROTO_UDP,
		.allocate = SOCKET_SERVER_ALLOCATE_EVERYTHING,
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

	for(;;){

		socketDetails *i;
		int j;

		// Handle UDP connections.
		// Loop forever here to handle the entire queue before continuing.
		for(;;){
			size_t nfds = testServerUDP.nfds;
			bufferSize = ssReceiveUDP(&testServerUDP, &i, buffer);
			// A new socket has just connected.
			if(testServerUDP.nfds != nfds){
				ssHandleConnectUDP(&testServerUDP, testServerUDP.detailsLast);
			}
			// The socket has sent some data.
			if(bufferSize >= 0){
				ssHandleBufferUDP(&testServerUDP, i, buffer, bufferSize);
			}else{
				break;
			}
		}


		// Handle TCP connections.
		i = testServerTCP.detailsLast;
		// Poll TCP connections.
		if((j = ssPollTCP(&testServerTCP)) < 0){
			// Fatal error.
			break;
		}
		// Check if a new client connected.
		if(testServerTCP.detailsLast != i){
			ssHandleConnectTCP(&testServerTCP, testServerTCP.detailsLast);
		}
		// Receive data from clients.
		i = testServerTCP.details+1;
		while(j > 0){
			if(sdValid(i)){

				if(flagsAreSet(i->handle->revents, POLLIN)){

					// Receives up to MAX_BUFFER_SIZE bytes of data from a client socket and stores it in buffer.
					bufferSize = recv(i->handle->fd, buffer, SOCKET_MAX_BUFFER_SIZE, 0);

					if(bufferSize == -1){
						// Error encountered, disconnect problematic socket.
						#ifdef SOCKET_DEBUG
						ssReportError("recv()", ssError);
						#endif
						ssHandleDisconnectTCP(&testServerTCP, i);
					}else if(bufferSize == 0){
						// If the buffer is empty, the connection has closed.
						ssHandleDisconnectTCP(&testServerTCP, i);
					}else{
						// Data received.
						ssHandleBufferTCP(&testServerTCP, i, buffer, bufferSize);
					}

					--j;

				}else if(flagsAreSet(i->handle->revents, POLLHUP)){
					// Hang up detected.
					ssHandleDisconnectTCP(&testServerTCP, i);
					--j;

				}

				++i;

			}
			--j;
		}

	}

	return 0;

}
