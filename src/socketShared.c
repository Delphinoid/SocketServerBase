#include "socketShared.h"
#include <stdio.h>

int ssGetAddressFamily(const char *ip){
	char buffer[16];
    if(inet_pton(AF_INET, ip, buffer)){
        return AF_INET;
    }else if(inet_pton(AF_INET6, ip, buffer)) {
        return AF_INET6;
    }
    return AF_UNSPEC;
}

void ssReportError(const char *failedFunction, int errorCode){
	printf("\nSocket function %s has failed: %i\nSee here for more information:\nhttps://msdn.microsoft.com/en-us/library/windows/desktop/ms740668%%28v=vs.85%%29.aspx\n\n",
	       failedFunction, errorCode);
}

void ssShutdownShared(){
	#ifdef _WIN32
		WSACleanup();
	#endif
}
