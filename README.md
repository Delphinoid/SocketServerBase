A very simple "base" for writing TCP / UDP socket servers in C.


It supports TCP, UDP and, with the magic of multithreading, both at once. A very basic example has been provided in main.c; just compile it with the following and it should be good to go:

g++ main.c inetPton.c socketVector.c socketShared.c socketTCP.c socketUDP.c -o SocketServerMulti.exe -lWs2_32


In order to use configs, just move config.txt to the folder containing the executable and modify it as you see fit. If you would like to add commands, you can do so in ssLoadConfig().