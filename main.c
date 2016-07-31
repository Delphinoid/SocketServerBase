#include "socketTCP.h"
#include "socketUDP.h"
#include <windows.h>

socketServer testServer;
HANDLE tcpThreadID;

void cleanup(){
	ssShutdown(&testServer);
	CloseHandle(tcpThreadID);
}

DWORD WINAPI handleTCP(){
	while(1)
		ssHandleConnectionsTCP(&testServer);
}

int main(int argc, char *argv[]){

	if(!ssInit(&testServer, INIT_TCP_AND_UDP, argc, (const char **)argv))  // Initialize the server
		return 1;
	tcpThreadID = CreateThread(NULL, 0, handleTCP, NULL, 0, NULL);
	atexit(cleanup);

	while(1)
		ssHandleConnectionsUDP(&testServer);

	return 0;

}
