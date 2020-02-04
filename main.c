#define WIN32_LEAN_AND_MEAN
#include "socket/socketTCP.h"
#include "socket/socketUDP.h"
#include <string.h>
#include <stdio.h>

socketServer testServerTCP;
socketServer testServerUDP;

return_t ssLoadConfig(char (*ip)[45], uint16_t *port, const int argc, char **argv){

	char *cfgPath = (char*)argv[0];
	cfgPath[strrchr(cfgPath, '\\') - cfgPath + 1] = '\0';  // Removes program name (everything after the last backslash) from the path
	strcpy(cfgPath + strlen(cfgPath), "config");  // Append "config" to the end

	FILE *serverConfig = fopen(cfgPath, "r");
	char lineFeed[1024];
	char *line;
	size_t lineLength;

	if(serverConfig != NULL){
		while(fgets(lineFeed, sizeof(lineFeed), serverConfig)){

			line = lineFeed;
			lineLength = strlen(line);

			// Remove new line and carriage return
			if(line[lineLength-1] == '\n'){
				line[--lineLength] = '\0';
			}
			if(line[lineLength-1] == '\r'){
				line[--lineLength] = '\0';
			}
			// Remove any comments from the line
			char *commentPos = strstr(line, "//");
			if(commentPos != NULL){
				lineLength -= commentPos-line;
				*commentPos = '\0';
			}
			// Remove any indentations from the line, as well as any trailing spaces and tabs
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
					strncpy(*ip, line+5, 40);

				// Port
				}else if(strncmp(line, "port = ", 7) == 0){
					*port = strtol(line+7, NULL, 0);

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

void ssHandleConnectTCP(socketServer *server, const socketHandle *handle, const socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Accepted TCP connection from %s (socket #%lu).\n", IP, (unsigned long)details->id);
}

void ssHandleBufferTCP(const socketServer *server, const socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Data received over TCP from %s (socket #%lu): %s\n", IP, (unsigned long)details->id, details->lastBuffer);
	ssSendDataTCP(ssGetSocketHandle(server, details->id), "Data received over TCP successfully. You should get this.\n");
}

void ssHandleDisconnectTCP(socketServer *server, const socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Closing TCP connection with %s (socket #%lu).\n", IP, (unsigned long)details->id);
	scdRemoveSocket(&server->connectionHandler, details->id);
}

void ssHandleConnectUDP(socketServer *server, const socketHandle *handle, const socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Accepted UDP connection from %s (socket #%lu).\n", IP, (unsigned long)details->id);
}

void ssHandleBufferUDP(const socketServer *server, const socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Data received over UDP from %s: %s\n", IP, details->lastBuffer);
	ssSendDataUDP(server, details, "Data received over UDP successfully. You might get this.\n");
}

void ssHandleDisconnectUDP(socketServer *server, const socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Closing UDP connection with %s (socket #%lu).\n", IP, (unsigned long)details->id);
	scdRemoveSocket(&server->connectionHandler, details->id);
}

void cleanup(){
	ssShutdownTCP(&testServerTCP);
	ssShutdownUDP(&testServerUDP);
	#ifdef _WIN32
		ssCleanup();
	#endif
}

int main(int argc, char *argv[]){

	size_t i;
	unsigned char flagsUDP = SOCK_ABSTRACT_HANDLE | SOCK_READ_FULL_QUEUE;
	unsigned char flagsTCP = 0;

	if(
		!ssStartup() ||
		!ssInit(&testServerTCP, SOCK_STREAM, IPPROTO_TCP, argc, argv, &ssLoadConfig) ||
		!ssInit(&testServerUDP, SOCK_DGRAM,  IPPROTO_UDP, argc, argv, &ssLoadConfig)
	){
		return 1;
	}

	atexit(cleanup);
	
	while(1){
		if(!ssHandleConnectionsUDP(&testServerUDP, 0, flagsUDP) ||
		   !ssHandleConnectionsTCP(&testServerTCP, 0, flagsTCP)){
			break;
		}
		for(i = 0; i < testServerTCP.connectionHandler.size; ++i){
			if((testServerTCP.connectionHandler.details[i].flags & SOCK_CONNECTED) > 0){
				// Socket has connected.
				ssHandleConnectTCP(&testServerTCP, &testServerTCP.connectionHandler.handles[i], &testServerTCP.connectionHandler.details[i]);
				testServerTCP.connectionHandler.details[i].flags &= ~SOCK_CONNECTED;
			}
			if((testServerTCP.connectionHandler.details[i].flags & SOCK_NEW_DATA) > 0){
				// Socket has sent data.
				ssHandleBufferTCP(&testServerTCP, &testServerTCP.connectionHandler.details[i]);
				testServerTCP.connectionHandler.details[i].flags &= ~SOCK_NEW_DATA;
			}
			if((testServerTCP.connectionHandler.details[i].flags & SOCK_DISCONNECTED) > 0 ||
			   (testServerTCP.connectionHandler.details[i].flags & SOCK_TIMED_OUT) > 0 ||
			   (testServerTCP.connectionHandler.details[i].flags & SOCK_ERROR) > 0){
				// Socket has disconnected.
				ssHandleDisconnectTCP(&testServerTCP, &testServerTCP.connectionHandler.details[i]);
				testServerTCP.connectionHandler.details[i].flags &= ~(SOCK_DISCONNECTED | SOCK_TIMED_OUT | SOCK_ERROR);
				--i;
			}
		}
		for(i = 0; i < testServerUDP.connectionHandler.size; ++i){
			if((testServerUDP.connectionHandler.details[i].flags & SOCK_CONNECTED) > 0){
				// Socket has connected.
				ssHandleConnectUDP(&testServerUDP, &testServerUDP.connectionHandler.handles[i], &testServerUDP.connectionHandler.details[i]);
				testServerUDP.connectionHandler.details[i].flags &= ~SOCK_CONNECTED;
			}
			if((testServerUDP.connectionHandler.details[i].flags & SOCK_NEW_DATA) > 0){
				// Socket has sent data.
				ssHandleBufferUDP(&testServerUDP, &testServerUDP.connectionHandler.details[i]);
				testServerUDP.connectionHandler.details[i].flags &= ~SOCK_NEW_DATA;
			}
			if((testServerUDP.connectionHandler.details[i].flags & SOCK_DISCONNECTED) > 0 ||
			   (testServerUDP.connectionHandler.details[i].flags & SOCK_TIMED_OUT) > 0 ||
			   (testServerUDP.connectionHandler.details[i].flags & SOCK_ERROR) > 0){
				// Socket has disconnected.
				ssHandleDisconnectUDP(&testServerUDP, &testServerUDP.connectionHandler.details[i]);
				testServerUDP.connectionHandler.details[i].flags &= ~(SOCK_DISCONNECTED | SOCK_TIMED_OUT | SOCK_ERROR);
				--i;
			}
		}
	}

	return 0;

}
