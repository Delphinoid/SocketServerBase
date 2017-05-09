#include "socketTCP.h"
#include "socketUDP.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>

socketTCP testServerTCP;
socketUDP testServerUDP;
HANDLE tcpThreadID;

void substringHelper(char *strTarget, char *strCopy, unsigned int pos, unsigned int length){
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

int loadConfig(char *ip, uint16_t *port, const int argc, const char *argv[]){

	char *cfgPath = (char*)argv[0];
	cfgPath[strrchr(cfgPath, '\\') - cfgPath + 1] = '\0';  // Removes program name (everything after the last backslash) from the path
	strcpy(cfgPath + strlen(cfgPath), "config.txt");  // Append "config.txt" to the end

	FILE *serverConfig = fopen(cfgPath, "r");
	char line[1000];
	char compare[8];
	char lineData[1000];

	if(serverConfig != NULL){
		while(!feof(serverConfig)){

			fgets(line, sizeof(line), serverConfig);

			// Remove any comments from the line
			char *commentPos = strstr(line, "//");
			if(commentPos != NULL){
				commentPos = '\0';
			}

			if(strlen(line) > 7){
				substringHelper(compare, line, 0, 5);
				if(strcmp(compare, "ip = ") == 0){
					substringHelper(lineData, line, 5, strlen(line) - 6);
					if(strlen(lineData) <= 39){
						strcpy(ip, lineData);
					}
				}
				substringHelper(compare, line, 0, 7);
				if(strcmp(compare, "port = ") == 0){
					substringHelper(lineData, line, 7, strlen(line) - 7);
					*port = strtol(lineData, NULL, 10);
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

void handleBufferTCP(socketTCP *server, unsigned int socketID){
	printf("Data received over TCP: %s\n", server->lastBuffer);
	ssSendDataTCP(server, socketID, "Data received over TCP successfully. You should get this.\n");
}

void handleDisconnectTCP(socketTCP *server, unsigned int socketID){
	printf("Closing TCP connection with socket #%i.\n", *((SOCKET*)cvGet(&server->connectedSockets, socketID)));
}

void handleBufferUDP(socketUDP *server, struct sockaddr *sender){
	printf("Data received over UDP: %s\n", server->lastBuffer);
	ssSendDataUDP(server, sender, "Data received over UDP successfully. You might get this.\n");
}

void handleDisconnectUDP(socketUDP *server){
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
		ssHandleConnectionsTCP(&testServerTCP, &handleBufferTCP, &handleDisconnectTCP);
}

int main(int argc, char *argv[]){

	if(!ssStartup() ||  // Initialize Winsock
	   !ssInitTCP(&testServerTCP, (const int)argc, (const char **)argv, &loadConfig) ||
	   !ssInitUDP(&testServerUDP, (const int)argc, (const char **)argv, &loadConfig))
		return 1;

	tcpThreadID = CreateThread(NULL, 0, handleTCP, NULL, 0, NULL);
	atexit(cleanup);

	while(1)
		ssHandleConnectionsUDP(&testServerUDP, &handleBufferUDP, &handleDisconnectUDP);

	return 0;

}
