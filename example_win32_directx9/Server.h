#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <wlanapi.h>
#include <iostream>
#include <thread>
#include <chrono>


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

    //Functions to start server:-
    void StartServer();
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

