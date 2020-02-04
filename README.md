A simple template for writing TCP / UDP socket servers in C. It can use select, and if the system supports it and SOCK_USE_POLL is defined in socketSettings.h, poll as well.

A very basic example has been provided in main.c and socketSettings.h; just compile it with the following (Ws2_32 must be linked on Windows) and it should be good to go:

gcc main.c inet_ntop.c inet_pton.c socketConnectionHandler.c socketServer.c socketShared.c socketTCP.c socketUDP.c -o SocketServerMulti.exe -lWs2_32