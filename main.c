#define WIN32_LEAN_AND_MEAN
#include "socketTCP.h"
#include "socketUDP.h"
#include <string.h>
#include <stdio.h>

socketServer testServerTCP;
socketServer testServerUDP;

unsigned char ssLoadConfig(char (*ip)[45], uint16_t *port, const int argc, const char *argv[]){

	char *cfgPath = (char*)argv[0];
	cfgPath[strrchr(cfgPath, '\\') - cfgPath + 1] = '\0';  // Removes program name (everything after the last backslash) from the path
	strcpy(cfgPath + strlen(cfgPath), "config.txt");  // Append "config.txt" to the end

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
	if(scdAddSocket(&server->connectionHandler, handle, details)){
		printf("Accepted TCP connection from %s (socket #%u)\n", IP, details->id);
	}else{
		printf("Error: could not accept TCP connection from %s: server is full.\n", IP);
	}
}

void ssHandleBufferTCP(const socketServer *server, const socketDetails *details){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Data received over TCP from %s (socket #%u): %s\n", IP, details->id, details->lastBuffer);
	ssSendDataTCP(ssGetSocketHandle(server, details->id), "Data received over TCP successfully. You should get this.\n");
}

void ssHandleDisconnectTCP(socketServer *server, const socketDetails *details, const char reason){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Closing TCP connection with %s (socket #%u).\n", IP, details->id);
	closesocket(ssGetSocketHandle(server, details->id)->fd);
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
	if(scdAddSocket(&server->connectionHandler, handle, details)){
		printf("Accepted UDP connection from %s (socket #%u)\n", IP, details->id);
	}else{
		printf("Error: could not accept UDP connection from %s: server is full.\n", IP);
	}
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

void ssHandleDisconnectUDP(socketServer *server, const socketDetails *details, const char reason){
	char IP[45];
	inet_ntop(details->address.ss_family,
	          (details->address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details->address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details->address)->sin6_addr)),
	          &IP[0],
	          sizeof(IP));
	printf("Closing UDP connection with %s (socket #%u).\n", IP, details->id);
	ssDisconnectSocketUDP(server, details->id);
}

void cleanup(){
	ssShutdownTCP(&testServerTCP);
	ssShutdownUDP(&testServerUDP);
	#ifdef _WIN32
		ssCleanup();
	#endif
}

int main(int argc, char *argv[]){

	if(!ssStartup() ||  // Initialize Winsock
	   !ssInit(&testServerTCP, SOCK_STREAM, IPPROTO_TCP, (const int)argc, (const char **)argv, &ssLoadConfig) ||
	   !ssInit(&testServerUDP, SOCK_DGRAM,  IPPROTO_UDP, (const int)argc, (const char **)argv, &ssLoadConfig))
		return 1;

	atexit(cleanup);

	unsigned char flagsUDP = SOCK_ABSTRACT_HANDLE | SOCK_READ_FULL_QUEUE;
	unsigned char flagsTCP = 0;
	while(1){
		if(!ssHandleConnectionsUDP(&testServerUDP, 0, &ssHandleConnectUDP, &ssHandleBufferUDP, &ssHandleDisconnectUDP, flagsUDP) ||
		   !ssHandleConnectionsTCP(&testServerTCP, 0, &ssHandleConnectTCP, &ssHandleBufferTCP, &ssHandleDisconnectTCP, flagsTCP)){
			break;
		}
	}

	return 0;

}
