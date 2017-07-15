#include "socketTCP.h"
#include "socketUDP.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>

#define MAX_SOCKETS 101

socketServer testServerTCP;
socketServer testServerUDP;
HANDLE tcpThreadID;

const char *inet_ntop(int af, const void *src, char *dst, size_t size);

unsigned char ssLoadConfig(char (*ip)[45], uint16_t *port, const int argc, const char *argv[]){

	char *cfgPath = (char*)argv[0];
	cfgPath[strrchr(cfgPath, '\\') - cfgPath + 1] = '\0';  // Removes program name (everything after the last backslash) from the path
	strcpy(cfgPath + strlen(cfgPath), "config.txt");  // Append "config.txt" to the end

	FILE *serverConfig = fopen(cfgPath, "r");
	char lineFeed[1024];
	char *line;
	char compare[1024];
	size_t lineLength;

	if(serverConfig != NULL){
		while(!feof(serverConfig)){

			fgets(lineFeed, sizeof(lineFeed), serverConfig);
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
			for(i = 0; (i < lineLength && !doneFront && !doneEnd); i++){
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
				if(strncpy(compare, line, 5) && (compare[5] = '\0') == 0 && strcmp(compare, "ip = ") == 0){
					strncpy(*ip, line+5, 40);

				// Port
				}else if(strncpy(compare, line, 7) && (compare[7] = '\0') == 0 && strcmp(compare, "port = ") == 0){
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

void ssHandleBufferTCP(socketServer *server, size_t socketID){
	socketHandle  handle  = *ssGetSocketHandle(server, socketID);
	socketDetails details = *ssGetSocketDetails(server, socketID);
	char recvIP[45];
	inet_ntop(details.address.ss_family,
	          (details.address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details.address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details.address)->sin6_addr)),
	          &recvIP[0],
	          sizeof(recvIP));
	printf("Data received over TCP from %s (socket #%i): %s\n", recvIP, handle.fd, server->lastBuffer);
	ssSendDataTCP(server, socketID, "Data received over TCP successfully. You should get this.\n");
}

void ssHandleDisconnectTCP(socketServer *server, size_t socketID){
	socketHandle  handle  = *ssGetSocketHandle(server, socketID);
	socketDetails details = *ssGetSocketDetails(server, socketID);
	char recvIP[45];
	inet_ntop(details.address.ss_family,
	          (details.address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details.address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details.address)->sin6_addr)),
	          &recvIP[0],
	          sizeof(recvIP));
	printf("Closing TCP connection with %s (socket #%i).\n", recvIP, handle.fd);
	ssDisconnectSocketTCP(server, socketID);
}

void ssHandleBufferUDP(socketServer *server, socketDetails details){
	char recvIP[45];
	inet_ntop(details.address.ss_family,
	          (details.address.ss_family == AF_INET ?
	          (void *)(&((struct sockaddr_in *)&details.address)->sin_addr) :
	          (void *)(&((struct sockaddr_in6 *)&details.address)->sin6_addr)),
	          &recvIP[0],
	          sizeof(recvIP));
	printf("Data received over UDP from %s: %s\n", recvIP, server->lastBuffer);
	ssSendDataUDP(server, details, "Data received over UDP successfully. You might get this.\n");
}

void ssHandleDisconnectUDP(socketServer *server){
	// Check for timeouts, if applicable
}

void cleanup(){
	ssShutdownTCP(&testServerTCP);
	ssShutdownUDP(&testServerUDP);
	ssCleanup();  // Terminate Winsock
	CloseHandle(tcpThreadID);
}

DWORD WINAPI handleTCP(){
	while(1)
		ssHandleConnectionsTCP(&testServerTCP, &ssHandleBufferTCP, &ssHandleDisconnectTCP);
}

int main(int argc, char *argv[]){

	if(!ssStartup() ||  // Initialize Winsock
	   !ssInit(&testServerTCP, SOCK_STREAM, IPPROTO_TCP, (const int)argc, (const char **)argv, &ssLoadConfig) ||
	   !ssInit(&testServerUDP, SOCK_DGRAM,  IPPROTO_UDP, (const int)argc, (const char **)argv, &ssLoadConfig))
		return 1;

	tcpThreadID = CreateThread(NULL, 0, handleTCP, NULL, 0, NULL);
	atexit(cleanup);

	while(1)
		ssHandleConnectionsUDP(&testServerUDP, &ssHandleBufferUDP, &ssHandleDisconnectUDP);

	return 0;

}
