
#include "Server.h"



std::string getLocalIP() {
    // Initialize Windows sockets
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Get adapter info
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD bufferSize = sizeof(adapterInfo);

    // Retrieve the list of adapters
    DWORD dwStatus = GetAdaptersInfo(adapterInfo, &bufferSize);
    if (dwStatus != ERROR_SUCCESS) {
        std::cerr << "Error getting adapter info!" << std::endl;
        return "";
    }

    // Loop through the adapters and display the IP address
    IP_ADAPTER_INFO* pAdapterInfo = adapterInfo;
    while (pAdapterInfo) {
        if (pAdapterInfo->IpAddressList.IpAddress.String[0] != '\0' &&
            (strcmp(pAdapterInfo->IpAddressList.IpAddress.String, "0.0.0.0") != 0)) {
            //std::cout << "Local IP Address: " << pAdapterInfo->IpAddressList.IpAddress.String << std::endl;
            std::string IP = pAdapterInfo->IpAddressList.IpAddress.String;
            return IP;
        }
        pAdapterInfo = pAdapterInfo->Next;
    }

    // Clean up
    WSACleanup();
    return "";
}




void Server::log(const std::string& message, int level)
{
    Applog.push_back({ message, level });
}

void Server::StartServer() {

    /*WSADATA wsaData;
    int wsaErr;

    WORD wVersionRequested = MAKEWORD(2, 2);*/

    wsaErr = WSAStartup(wVersionRequested, &wsaData);

    if (wsaErr != 0) {
        std::cout << "The Winsock dll not found ! " << std::endl;
        return;
    }
    else {
        std::cout << "The Winsock dll found ! " << std::endl;
        std::cout << "The status : " << wsaData.szSystemStatus << std::endl;
    }

    mySocket = INVALID_SOCKET;
    mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mySocket == INVALID_SOCKET) {

        std::cout << "Error at socket Creation" << WSAGetLastError() << std::endl;
        ServerCleanup();
        return;
    }
    else {
        std::cout << "socket() is ok!" << std::endl;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;

    localIP = getLocalIP();

    if (localIP.empty()) {
        std::cout << "No valid local IP address found!" << std::endl;
        return;
    }

    // Convert std::string to std::wstring (wide string)
    std::wstring wideIP(localIP.begin(), localIP.end());



    if (InetPtonW(AF_INET, wideIP.c_str(), &service.sin_addr) != 1) {
        std::cout << "InetPton() failed: " << WSAGetLastError() << std::endl;
        ServerCleanup();
        return;
    }
    service.sin_port = htons(Port);

    if (bind(mySocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind() failed" << WSAGetLastError() << std::endl;
        ServerCleanup();
        return;
    }
    else {
        std::cout << "bind is ok!" << std::endl;
    }

}

void Server::ServerCleanup()
{
    if (mySocket != INVALID_SOCKET) {
        closesocket(mySocket);
    }
    WSACleanup();

}


void Server::Listen()
{
    if (listen(mySocket, 1) == SOCKET_ERROR) {
        std::cout << "listen(): Error listening on socket" << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "listen(): is ok ,waiting for connections..." << std::endl;
    }
}
/*
bool Server::Accept()
{
    u_long mode = 1;
    ioctlsocket(mySocket, FIONBIO, &mode);
    clientSocketSize = sizeof(ClientSocket);
    acceptSocket = accept(mySocket, (SOCKADDR*)&ClientSocket, &clientSocketSize);

    if (acceptSocket == INVALID_SOCKET) {
        std::cout << "accept failed:" << WSAGetLastError() << std::endl;
        return false;
    }

    inet_ntop(AF_INET, &(ClientSocket.sin_addr), ClientIP, INET_ADDRSTRLEN);
    std::cout << "Client IP Address : " << ClientIP << std::endl;

  
    return true;
}
*/

bool Server::Accept()
{
    clientSocketSize = sizeof(ClientSocket);
    acceptSocket = accept(mySocket, (SOCKADDR*)&ClientSocket, &clientSocketSize);

    if (acceptSocket == INVALID_SOCKET) {
        std::cout << "accept failed:" << WSAGetLastError() << std::endl;
        return false;
    }

    inet_ntop(AF_INET, &(ClientSocket.sin_addr), ClientIP, INET_ADDRSTRLEN);
    std::cout << "Client IP Address : " << ClientIP << std::endl;


    
    return true;
}

void Server::StartAcceptingConnections()
{
    std::thread acceptThread([this]() {
        while (true)
        {
            clientSocketSize = sizeof(ClientSocket);
            acceptSocket = accept(mySocket, (SOCKADDR*)&ClientSocket, &clientSocketSize);

            if (acceptSocket == INVALID_SOCKET)
            {
                int error = WSAGetLastError();

                // Non-blocking sockets return WSAEWOULDBLOCK if no client is ready to connect
                if (error == WSAEWOULDBLOCK)
                {
                    // No pending connections — continue checking
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Prevent busy-waiting
                    continue;
                }

                std::cout << "accept failed: " << error << std::endl;
                break;  // Fatal error, break the loop
            }

            // Successful connection
            inet_ntop(AF_INET, &(ClientSocket.sin_addr), ClientIP, INET_ADDRSTRLEN);
            std::cout << "Client connected!!!!! IP: " << ClientIP << std::endl;
            isClientConnected = true;
            // Perform further client-specific logic here (e.g., storing socket, sending data)
        }
        });

    acceptThread.detach();  // Detach allows it to run independently
}




void Server::CloseServer()
{
    ServerCleanup();
}


void Server::InitSocket()
{
    int sockfd = mySocket;
    u_long mode = 1; // 1 to enable non-blocking, 0 to disable
    ioctlsocket(sockfd, FIONBIO, &mode);

}

bool Server::ConnectServer(std::string IP) {
    WSADATA wsaData;
    int wsaErr;

    WORD wVersionRequested = MAKEWORD(2, 2);

    wsaErr = WSAStartup(wVersionRequested, &wsaData);

    if (wsaErr != 0) {
        std::cout << "The Winsock dll not found ! " << std::endl;
        return false;
    }
    else {
        std::cout << "The Winsock dll found ! " << std::endl;
        std::cout << "The status : " << wsaData.szSystemStatus << std::endl;
    }

    mySocket = INVALID_SOCKET;
    mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mySocket == INVALID_SOCKET) {

        std::cout << "Error at socket():" << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
    else {
        std::cout << "socket() is ok!" << std::endl;
    }

    sockaddr_in ClientService;
    ClientService.sin_family = AF_INET;

    std::wstring wideIP(IP.begin(), IP.end());

    if (InetPtonW(AF_INET, wideIP.c_str(), &ClientService.sin_addr) != 1) {
        std::cout << "InetPton() failed: " << WSAGetLastError() << std::endl;
        closesocket(mySocket);
        WSACleanup();
        return false;
    }
    ClientService.sin_port = htons(Port);

    if (connect(mySocket, (SOCKADDR*)&ClientService, sizeof(ClientService)) == SOCKET_ERROR) {
        std::cout << "Client connect() - Failed to connect" << std::endl;
        closesocket(mySocket);
        WSACleanup();
        return false;
    }
    else {
        std::cout << "Client Connect(): is OK." << std::endl;
        std::cout << "Client: Can start sending and Recieving data..." << std::endl;
    }

    //char buffer[200];
    /*int RUN = 1;*/
    //std::cin.ignore();
    /*while (RUN) {


        printf("Enter your message : ");
        std::cin.getline(buffer, 200);
        if (strcmp(buffer, "over") == 0) {
            RUN = 0;
        }
        int byteCount = send(mySocket, buffer, 200, 0);
        if (byteCount == SOCKET_ERROR) {
            printf("Server send error %ld.\n", WSAGetLastError());

            closesocket(mySocket);
            WSACleanup();
            return false;
        }
        else {
            printf("Server sent %ld bytes \n", byteCount);
        }



    }*/

    //closesocket(mySocket);
    //WSACleanup();
    u_long mode = 1;
    ioctlsocket(mySocket, FIONBIO, &mode);

    return true;
}

void Server::ClientCleanup()
{
    closesocket(mySocket);
    WSACleanup();
}

bool Server::SendMessageToOther(SOCKET* sock ,char* message)
{
    /*char buffer[200];
    std::cin.ignore();

    printf("Enter your message : ");
    std::cin.getline(buffer, 200);*/


    int byteCount = send((*sock), message, 200, 0); //can change size of byte send later


    if (byteCount == SOCKET_ERROR) {
        printf("Server send error %ld.\n", WSAGetLastError());

        closesocket(*sock);
        WSACleanup();
        return false;
    }

    else {
        printf("Server sent %ld bytes \n", byteCount);
    }




    return true;
}



void startReceiving(SOCKET* sock, char* message,bool* status) {

    while (true) {
        char receiveBuffer[200] = { 0 };
        int byteCount = recv((*sock), receiveBuffer, sizeof(receiveBuffer) - 1, 0);
        std::cout << "check for if recv is blocking" << std::endl;
        if (byteCount > 0) {
            // Ensure null termination
            receiveBuffer[byteCount] = '\0';

            // Copy only if message is within bounds
            if (byteCount < static_cast<int>(sizeof(receiveBuffer))) {
                strcpy_s(message, 200, receiveBuffer);
                std::cout << "Received message: " << receiveBuffer << std::endl;
                 *status=true;
            }
            else {
                std::cerr << "Message too long to handle." << std::endl;
                *status=false;
            }
        }
        else if (byteCount == 0) {
            std::cout << "Client Disconnected!" << std::endl;
            *status=false;
            break;
        }
        else {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                printf("Receive error: %d\n", err);
                *status=false;
                break;
            }
        }

        
        Sleep(500);
    }
    
}




bool Server::ReceiveMessageFromOther(SOCKET* sock, char* message, bool* RecieveStatus) {

    std::thread receiveThread(startReceiving, sock, message,RecieveStatus);
    receiveThread.detach();
    return true;
}



