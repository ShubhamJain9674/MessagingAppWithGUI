#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <wlanapi.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

struct Log {
    std::string message;
    int level{ 0 };
};

std::string getLocalIP();


class Server {

private:

    
    std::string localIP;
    WSADATA wsaData;
    int wsaErr;

    WORD wVersionRequested = MAKEWORD(2, 2);


public:

    
    
    SOCKET acceptSocket;
    sockaddr_in ClientSocket;
    int clientSocketSize;
    char ClientIP[INET_ADDRSTRLEN];


    int Port = 55555;


    
    std::string ConnectedDeviceIP = "";
    bool ConnectionStatus{ false };

    SOCKET mySocket{ INVALID_SOCKET };
    std::atomic<bool> isClientConnected = false;
    std::atomic<bool> acceptFailed = false;

    std::vector<Log> Applog;

    //function to push message to log:-
    void log(const std::string& message,int level);

    //Functions to start server:-
    bool StartServer();
    void ServerCleanup();
    void Listen();
    bool Accept();
    void CloseServer();



    //Client Function code:-
    void InitSocket();
    bool ConnectServer(std::string IP);
    void ClientCleanup();
    bool SendMessageToOther(SOCKET* sock,char* message);
    bool ReceiveMessageFromOther(SOCKET* sock, char* message,bool* RecieveStatus);
    void StartAcceptingConnections();


    //1 - client , 2 -server
    int connectionType = -1;
    SOCKET MSGsock=INVALID_SOCKET;
};

