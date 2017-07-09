A simple template for writing TCP / UDP socket servers in C. Currently I need to add a system for recognizing UDP connections from the same user and handling timeouts.


It supports TCP, UDP and, with the magic of multithreading, both at once. A very basic example for Windows has been provided in main.c; just compile it with the following and it should be good to go:

g++ main.c inetNtop.c inetPton.c cVector.c socketServer.c socketTCP.c socketUDP.c -o SocketServerMulti.exe -lWs2_32