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
#include <fstream>
#include <direct.h> // for _mkdir on Windows
#include <sys/stat.h>

#define DT_Message 1
#define DT_BinaryFile 2
#define DT_endFlag 3
#define DT_FileHeader 4

#define RBuffAmount 3 * 1024 * 1024


struct Log {
    std::string message;
    int level{ 0 };
};

std::string getLocalIP();


struct DataPacket {

    std::string SenderName = "";
    std::string FileName = "";
    int DataType = -1;
    int DataSize = 0;
    int PacketID = -1;
    int totalPackets = -1;
    std::vector<char> Data;
    std::string CheckSum = "";

};


class Server {

private:

    
    std::string localIP;
    WSADATA wsaData;
    int wsaErr;

    WORD wVersionRequested = MAKEWORD(2, 2);

    

    



public:

    int MaxPacketDataSize = 1024*1024; //1kB
    
    
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
    bool SendFileToOther(SOCKET* sock, std::string filepath);
    
    void StartAcceptingConnections();

    //file sharing
    DataPacket CreateMessageDataPacket(std::string Message);
    DataPacket GetHeaderDataPacket(std::string filepath);
    DataPacket GetNextFilePacket(std::string filepath, int PacketID);
    

    std::string GetFileNameFromPath(const std::string& filepath);

    bool SendingFile{ false };
    bool ReceivingFile{ false };
    std::string sendingFileName;
    float sentProgress = 0.0f;
    int DataTransferred = 0;
    int fileSize = 0;

    //1 - client , 2 -server
    int connectionType = -1;
    SOCKET MSGsock=INVALID_SOCKET;

    DataPacket ReceivingFileHeader;
    
};

