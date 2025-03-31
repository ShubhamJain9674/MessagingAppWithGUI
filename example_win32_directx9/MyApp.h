#pragma once

#define _WINSOCKAPI_ 


#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <wlanapi.h>
#include "Server.h"

#include <iostream>
//#include <cstring>
#include <sstream>

#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#include <thread>



namespace MyApp {

    void TopBar(bool*,Server*);
    void GetIpWindow(char*,bool*,Server*);
    void MessageBoxWin(bool*, Server*,char*,char*);
    void Console(Server* MyServer);
}
