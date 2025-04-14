
#include "Server.h"



std::vector<char> SerializeDataPacket(const DataPacket& DPack) {

    std::vector<char> buffer;

    // Serialize SenderName (null-terminated string)
    buffer.insert(buffer.end(), DPack.SenderName.begin(), DPack.SenderName.end());
    buffer.push_back('\0');  // Null terminator

    // Serialize FileName (null-terminated string)
    buffer.insert(buffer.end(), DPack.FileName.begin(), DPack.FileName.end());
    buffer.push_back('\0');  // Null terminator

    // Serialize integer fields (DataType, DataSize, PacketID, totalPackets)
    buffer.push_back(static_cast<char>(DPack.DataType)); // Serialize DataType
    buffer.push_back(static_cast<char>(DPack.DataSize)); // Serialize DataSize
    buffer.push_back(static_cast<char>(DPack.PacketID)); // Serialize PacketID
    buffer.push_back(static_cast<char>(DPack.totalPackets)); // Serialize totalPackets

    // Serialize Data (message or file data)
    buffer.insert(buffer.end(), DPack.Data.begin(), DPack.Data.end());

    return buffer;  // Return the serialized byte buffer
}

DataPacket DeserializeDataPacket(const std::vector<char>& buffer) {
    DataPacket DPack;
    size_t index = 0;

    // Deserialize SenderName (until null terminator)
    while (buffer[index] != '\0') {
        DPack.SenderName.push_back(buffer[index]);
        index++;
    }
    index++;  // Skip null terminator

    // Deserialize FileName (until null terminator)
    while (buffer[index] != '\0') {
        DPack.FileName.push_back(buffer[index]);
        index++;
    }
    index++;  // Skip null terminator

    // Deserialize integer fields (DataType, DataSize, PacketID, totalPackets)
    DPack.DataType = static_cast<int>(buffer[index++]);
    DPack.DataSize = static_cast<int>(buffer[index++]);
    DPack.PacketID = static_cast<int>(buffer[index++]);
    DPack.totalPackets = static_cast<int>(buffer[index++]);

    // Deserialize Data (message or file data)
    while (index < buffer.size()) {
        DPack.Data.push_back(buffer[index++]);
    }

    return DPack;  // Return the deserialized DataPacket
}

std::string GetFileNameFromPath(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos)
        return filepath.substr(pos + 1);
    return filepath; // If no slash, whole string is filename
}

DataPacket Server::CreateMessageDataPacket(std::string Message) {

    DataPacket DPack;
    DPack.SenderName = localIP;
    DPack.DataType = DT_Message;
    DPack.DataSize = Message.size();

    DPack.Data = std::vector<char>(Message.begin(), Message.end());
    DPack.Data.push_back('\0');

    return DPack;

}


DataPacket Server::GetHeaderDataPacket(std::string filepath) {

    DataPacket DPack;
    DPack.SenderName = localIP;
    DPack.FileName = GetFileNameFromPath(filepath);
    DPack.DataType = DT_FileHeader;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.close();

    DPack.DataSize = static_cast<int>(size);
    DPack.PacketID = 0;
    DPack.totalPackets = (size + MaxPacketDataSize - 1) / MaxPacketDataSize;

    return DPack;
}



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

bool Server::StartServer() {

    /*WSADATA wsaData;
    int wsaErr;

    WORD wVersionRequested = MAKEWORD(2, 2);*/

    wsaErr = WSAStartup(wVersionRequested, &wsaData);

    if (wsaErr != 0) {
        std::cout << "The Winsock dll not found ! " << std::endl;
        log("The winsock dll not found !", 2);
        ServerCleanup();
        return false;
    }
    else {
        std::cout << "The Winsock dll found ! " << std::endl;
        std::cout << "The status : " << wsaData.szSystemStatus << std::endl;

        log("winsock dll found ! ", 0);
        log(std::string("status : ") + wsaData.szSystemStatus, 0);
    }

    mySocket = INVALID_SOCKET;
    mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mySocket == INVALID_SOCKET) {

        std::cout << "Error at socket Creation" << WSAGetLastError() << std::endl;
        log("Error at socket Creation !", 2);
        ServerCleanup();
        return false;
    }
    else {
        std::cout << "socket() is ok!" << std::endl;
        log("Socket creation successful!",0);
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;

    localIP = getLocalIP();

    if (localIP.empty()) {
        std::cout << "No valid local IP address found!" << std::endl;
        log("No Valid Local IP address found !", 1);
        ServerCleanup();
        return false;
    }

    // Convert std::string to std::wstring (wide string)
    std::wstring wideIP(localIP.begin(), localIP.end());



    if (InetPtonW(AF_INET, wideIP.c_str(), &service.sin_addr) != 1) {
        std::cout << "InetPton() failed: " << WSAGetLastError() << std::endl;
        log(std::string("InetPton failed : " + WSAGetLastError()), 2);
        ServerCleanup();
        return false;
    }
    service.sin_port = htons(Port);

    if (bind(mySocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind() failed" << WSAGetLastError() << std::endl;
        log(std::string("bind failed" + WSAGetLastError()), 2);

        ServerCleanup();
        return false;
    }
    else {
        std::cout << "bind is ok!" << std::endl;
        log("bind is ok!", 0);


    }
    return true;

}

void Server::ServerCleanup()
{
    if (mySocket != INVALID_SOCKET) {
        closesocket(mySocket);
    }
    WSACleanup();
    ConnectionStatus = false;
    connectionType = -1;
    ConnectedDeviceIP = "";

}


void Server::Listen()
{
    if (listen(mySocket, 1) == SOCKET_ERROR) {
        std::cout << "listen(): Error listening on socket" << WSAGetLastError() << std::endl;
        log("listen(): Error listening on socket", 2);
    }
    else {
        std::cout << "listen(): is ok ,waiting for connections..." << std::endl;
        log("listen(): is ok ,waiting for connections...", 0);
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
        log("accept failed!", 2);
        return false;
    }

    inet_ntop(AF_INET, &(ClientSocket.sin_addr), ClientIP, INET_ADDRSTRLEN);
    std::cout << "Client IP Address : " << ClientIP << std::endl;
    log(std::string("client IP address : ") + ClientIP, 0);

    
    return true;
}

void Server::StartAcceptingConnections()
{
    acceptFailed = false;
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
                log("accept failed!", 2);
                acceptFailed = true;
                ServerCleanup();
                break;  // Fatal error, break the loop
            }

            // Successful connection
            inet_ntop(AF_INET, &(ClientSocket.sin_addr), ClientIP, INET_ADDRSTRLEN);
            std::cout << "Client connected!!!!! IP: " << ClientIP << std::endl;
            log("client Connected", 0);
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
        log("winsock dll not found!", 1);
        return false;
    }
    else {
        std::cout << "The Winsock dll found ! " << std::endl;
        std::cout << "The status : " << wsaData.szSystemStatus << std::endl;

        log("winsock dll found!",0);
        log(std::string("status : ") + wsaData.szSystemStatus, 0);
    }

    mySocket = INVALID_SOCKET;
    mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mySocket == INVALID_SOCKET) {

        std::cout << "Error at socket():" << WSAGetLastError() << std::endl;
        log(std::string("Error at socket() : " + WSAGetLastError()),2);

        WSACleanup();
        return false;
    }
    else {
        std::cout << "socket() is ok!" << std::endl;
        log("socket() is ok !", 0);
    }

    sockaddr_in ClientService;
    ClientService.sin_family = AF_INET;

    std::wstring wideIP(IP.begin(), IP.end());

    if (InetPtonW(AF_INET, wideIP.c_str(), &ClientService.sin_addr) != 1) {

        std::cout << "InetPton() failed: " << WSAGetLastError() << std::endl;
        log(std::string("InetPton() failed : " + WSAGetLastError()), 2);

        closesocket(mySocket);
        WSACleanup();
        return false;
    }
    ClientService.sin_port = htons(Port);

    if (connect(mySocket, (SOCKADDR*)&ClientService, sizeof(ClientService)) == SOCKET_ERROR) {

        std::cout << "Client connect() - Failed to connect" << std::endl;
        log("Client Connect() - failed to connect", 2);

        closesocket(mySocket);
        WSACleanup();
        return false;
    }
    else {
        std::cout << "Client Connect(): is OK." << std::endl;
        std::cout << "Client: Can start sending and Recieving data..." << std::endl;

        log("Client Connect is ok!", 0);
        log("Client can start sending and recieving data...", 0);
        ConnectionStatus = true;
        connectionType = 1;
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
    ConnectionStatus = true;

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

    DataPacket DPack= CreateMessageDataPacket(message);
    
    
    std::vector<char> serializedData = SerializeDataPacket(DPack);
    int byteCount = send((*sock), serializedData.data(), serializedData.size(), 0); //can change size of byte send later


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


bool Server::SendFileToOther(SOCKET* sock, std::string filepath) {

    //code for sending First Header file:-

    DataPacket HeaderDataPacket = GetHeaderDataPacket(filepath);
    std::vector<char> serializedData = SerializeDataPacket(HeaderDataPacket);

    int byteCount = send((*sock), serializedData.data(), serializedData.size(), 0);

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



void startReceiving(SOCKET* sock, char* message,bool* status,Server* MyServer) {

    while (true) {
        std::vector<char> receiveBuffer(512);

        int byteCount = recv((*sock), receiveBuffer.data(),receiveBuffer.size(), 0);
        //std::cout << "check for if recv is blocking" << std::endl;
        
        
        if (byteCount > 0) {
            // Ensure null termination
            receiveBuffer.resize(byteCount);
            try {

                DataPacket DPack = DeserializeDataPacket(receiveBuffer);

                // Copy only if message is within bounds
                if (DPack.DataSize <= 256) {

                    switch (DPack.DataType) {

                        case DT_Message:
                        {


                            std::string receivedMessage(DPack.Data.begin(), DPack.Data.end());

                            memcpy(message,receivedMessage.c_str(),DPack.DataSize);
                            /*strcpy_s(message, 200, receiveBuffer);*/
                            message[DPack.DataSize] = '\0';

                            std::cout << "Received message: " << message << std::endl;


                            MyServer->log(std::string("Received Message : ") + message, 0);

                            *status = true;

                            break;
                        }
                        case DT_FileHeader:

                            std::cout << "received header file!" << std::endl;
                            std::cout << "Sender name : " <<  DPack.SenderName << std::endl;
                            std::cout << "FileName : " << DPack.FileName << std::endl;
                            std::cout << "Data type : " << DPack.DataType << std::endl;
                            std::cout << "Data size : " << DPack.DataSize << std::endl;
                            std::cout << "Packet ID : " << DPack.PacketID << std::endl;
                            std::cout << "TotalNumberOfPackets : " << DPack.totalPackets << std::endl;


                            break;


                    }

    
                    







                }
                else {
                    std::cerr << "Message too long to handle." << std::endl;
                    MyServer->log("Message too long to handle.", 1);
                    *status = false;
                }

            }
            catch (const std::exception& e) {
                std::cerr << "Error during deserialization: " << e.what() << std::endl;
                MyServer->log("Deserialization Error", 2);
                *status = false;
            }
        }
        else if (byteCount == 0) {
                std::cout << "Client Disconnected!" << std::endl;
                MyServer->log("Client Disconnected!", 1);
                *status=false;
                break;
            }
            else {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK) {
                    printf("Receive error: %d\n", err);
                    MyServer->log("Receive Error : " + err,2);
                    *status=false;
                    break;
                }
            }

        
        
        Sleep(500);
    }
    
}




bool Server::ReceiveMessageFromOther(SOCKET* sock, char* message, bool* RecieveStatus) {

    std::thread receiveThread(startReceiving, sock, message,RecieveStatus,this);
    receiveThread.detach();
    return true;
}



