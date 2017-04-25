A simple template for writing TCP / UDP socket servers in C.


It supports TCP, UDP and, with the magic of multithreading, both at once. A very basic example has been provided in main.c; just compile it with the following and it should be good to go:

g++ main.c inetPton.c cVector.c socketShared.c socketTCP.c socketUDP.c -o SocketServerMulti.exe -lWs2_32