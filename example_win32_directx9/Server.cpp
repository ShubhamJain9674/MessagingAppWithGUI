
#include "Server.h"




std::string GetExecutableDirA() {
    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(
        /* hModule = */ nullptr,
        /* lpFilename = */ path,
        /* nSize = */ MAX_PATH
    );
    if (len == 0 || len == MAX_PATH) {
        // error or truncated
        return "";
    }

    // Find the last backslash and chop off the filename
    std::string full(path, len);
    size_t pos = full.find_last_of("\\/");
    if (pos == std::string::npos) {
        return "";  // unexpected
    }
    return full.substr(0, pos);
}




void SerializeDataPacket(DataPacket* packet, char* outBuffer, int bufferSize)
{
    memset(outBuffer, 0, bufferSize); // Optional: zero out

    int index = 0;
    memcpy(outBuffer + index, packet->SenderName, sizeof(packet->SenderName));
    index += sizeof(packet->SenderName);

    memcpy(outBuffer + index, packet->FileName, sizeof(packet->FileName));
    index += sizeof(packet->FileName);

    memcpy(outBuffer + index, &packet->DataType, sizeof(int));
    index += sizeof(int);

    memcpy(outBuffer + index, &packet->DataSize, sizeof(int));
    index += sizeof(int);

    memcpy(outBuffer + index, &packet->PacketID, sizeof(int));
    index += sizeof(int);

    memcpy(outBuffer + index, &packet->totalPackets, sizeof(int));
    index += sizeof(int);

    memcpy(outBuffer + index, packet->Data, sizeof(packet->Data));
    index += sizeof(packet->Data);

    memcpy(outBuffer + index, packet->CheckSum, sizeof(packet->CheckSum));
}

DataPacket* DeserializeDataPacket(const char* buffer, int length) {
    if (length != sizeof(DataPacket)) {
        return nullptr;  // Corrupted or unexpected size
    }

    DataPacket* DPack = (DataPacket*)malloc(sizeof(DataPacket));
    if (DPack == nullptr) {
        return nullptr;
    }

    memcpy(DPack, buffer, sizeof(DataPacket));
    return DPack;
}




std::string Server::GetFileNameFromPath(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos)
        return filepath.substr(pos + 1);
    return filepath; // If no slash, whole string is filename
}

DataPacket* Server::CreateMessageDataPacket(std::string Message) {

    DataPacket* DPack=(DataPacket*)malloc(sizeof(DataPacket));

    if (DPack == nullptr) {
        std::cout << "Memory allocation failed at Create Message DataPacket () ! " << std::endl;
        return nullptr;
    }
    memset(DPack, 0, sizeof(DataPacket));

    strncpy(DPack->SenderName,localIP.c_str(),localIP.length());
    DPack->SenderName[localIP.length()] = '\0';

    DPack->DataType = DT_Message;
    DPack->DataSize = Message.size();

    //DPack->Data = std::vector<char>(Message.begin(), Message.end());
    strncpy(DPack->Data, Message.c_str(), Message.length());
    DPack->Data[sizeof(DPack->Data)-1] = '\0';

    return DPack;

}


DataPacket* Server::GetHeaderDataPacket(std::string filepath) {

    DataPacket* DPack=(DataPacket*)malloc(sizeof(DataPacket)) ;
    if (DPack == nullptr) {
        std::cout << "Header Data Pack Memory Allocation Failed ! " << std::endl;
        return NULL;
    }
    memset(DPack, 0, sizeof(DataPacket));

    strncpy(DPack->SenderName,localIP.c_str(), localIP.length());
    DPack->SenderName[sizeof(DPack->SenderName)-1] = '\0';

    std::string FileName = GetFileNameFromPath(filepath);
    strncpy(DPack->FileName,FileName.c_str(),FileName.length());
    DPack->FileName[sizeof(DPack->FileName)-1] = '\0';


    DPack->DataType = DT_FileHeader;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.close();

    DPack->DataSize = static_cast<int>(size);
    DPack->PacketID = -1;
    DPack->totalPackets = (size + MaxPacketDataSize - 1) / MaxPacketDataSize;

    return DPack;
}

DataPacket* Server::GetNextFilePacket(std::string filepath, int PacketID) {

    DataPacket* DPack=(DataPacket*)malloc(sizeof(DataPacket)) ;
    if (DPack == nullptr) {
        std::cout << "Memory allocation failed at GetNextFilePacket" << std::endl;
        return nullptr;
    }
    memset(DPack, 0, sizeof(DataPacket));

    strncpy(DPack->SenderName, localIP.c_str(), localIP.length());
    DPack->SenderName[sizeof(DPack->SenderName)-1] = '\0';


    std::string filename = GetFileNameFromPath(filepath);
    strncpy(DPack->FileName,filename.c_str(),filename.length());
    DPack->FileName[sizeof(DPack->FileName)-1] = '\0';

    DPack->DataType = DT_BinaryFile;

    /*DPack.DataSize = MaxPacketDataSize;*/
    DPack->PacketID = PacketID;
    DPack->totalPackets = 1;
    //DPack.CheckSum = "";

    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return nullptr; // Return empty/invalid packet if file can't be opened
    }

    std::streampos offset = static_cast<std::streampos>(PacketID) * MaxPacketDataSize;
    file.seekg(offset);
    if (file.eof()) {
        std::cerr << "Reached end of file or invalid PacketID: " << PacketID << std::endl;
        return nullptr;
    }

    file.read(DPack->Data, MaxPacketDataSize);
    std::streamsize bytesRead = file.gcount();
    DPack->DataSize = static_cast<int>(bytesRead);


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
   

    DataPacket* DPack= CreateMessageDataPacket(message);
    if (DPack == nullptr) {
        std::cout << "Got Null ptr from CreateMessageDataPacket" << std::endl;
        log("Got Null ptr from CreateMessageDataPacket", 2);
        log("message failed", 2);
        return false;
    }
    int SerializeDataSize = sizeof(DataPacket);
    
    char* SerializedData=(char*)malloc(sizeof(DataPacket));
    if (SerializedData == nullptr) {
        std::cout << "Memory Allocation failed in SendMessageToOther()! " << std::endl;
        return false;
    }

    SerializeDataPacket(DPack,SerializedData,SerializeDataSize);

    if (sock == nullptr) {
        std::cout << "NULL SOCKET Pointer at send message !" << std::endl;
        return false;
    }
    sendMutex.lock();
    
    int byteCount = send((*sock), SerializedData,sizeof(DataPacket), 0); //can change size of byte send later

    sendMutex.unlock();

    if (byteCount == SOCKET_ERROR) {
        printf("Server send error %ld.\n", WSAGetLastError());
        printf("SOCKET ERROR IN send message\n");
        closesocket(*sock);
        WSACleanup();
        return false;
    }
   

    else {
        printf("Server sent %ld bytes \n", byteCount);
    }
    
    
    
    free(DPack);
    free(SerializedData);
    
    return true;
}

void FreeDataPack(DataPacket* DPack) {

    DataPacket* temp = DPack;
    DPack = nullptr;
    if (temp != nullptr) {
        free(temp);
    }

}


void SendFile(SOCKET* sock, std::string filepath,Server* myServer) {

    //code for sending First Header file:-
    int HeaderDataPacketSize=sizeof(DataPacket);

    DataPacket* HeaderDataPacket = myServer->GetHeaderDataPacket(filepath);

    char* SerializedHeaderData=(char*)malloc(sizeof(DataPacket));
    if (SerializedHeaderData == nullptr) {
        std::cout << "Memory Allocation failed in SendFile()! " << std::endl;
        return;
    }


    SerializeDataPacket(HeaderDataPacket,SerializedHeaderData,HeaderDataPacketSize);
    myServer->sendMutex.lock();
    int byteCount = send((*sock),SerializedHeaderData,sizeof(DataPacket),0);
    myServer->sendMutex.unlock();

    if (byteCount == SOCKET_ERROR) {
        printf("Server send error %ld.\n", WSAGetLastError());

        closesocket(*sock);
        WSACleanup();
    }

    else {
        printf("Server sent %ld bytes \n", byteCount);
    }

   


    DataPacket* FileDataPacket=nullptr;
    myServer->fileSize += HeaderDataPacket->DataSize;
    char* SerializedData=(char*)malloc(sizeof(DataPacket));
    if (SerializedData == nullptr) {
        std::cout << "Memory Allocation failed in SendFile()! " << std::endl;
        return;
    }

    int SerializedDataSize = sizeof(DataPacket);

    for (int i = 0; i < HeaderDataPacket->totalPackets; i++) {

        FileDataPacket=myServer->GetNextFilePacket(filepath,i);
        myServer->sentProgress = static_cast<float>(i)/ (HeaderDataPacket->totalPackets-1);
        myServer->DataTransferred += FileDataPacket->DataSize;

        myServer->sendMutex.lock();

        SerializeDataPacket(FileDataPacket,SerializedData,SerializedDataSize);
        int byteCount = send((*sock), SerializedData,sizeof(DataPacket), 0);

        if (byteCount == SOCKET_ERROR) {
            printf("Server send error %ld.\n", WSAGetLastError());

            closesocket(*sock);
            WSACleanup();
        }

        else {
            printf("Server sent %ld bytes \n", byteCount);
        }

        myServer->sendMutex.unlock();
        Sleep(500);


    }

    myServer->SendingFile = false;
    myServer->sendingFileName = "";
    float sentProgress = 0.0f;
    int DataTransferred = 0;
    int fileSize = 0;
    free(SerializedData);
    free(SerializedHeaderData);

    free(HeaderDataPacket);
    free(FileDataPacket);
}

bool Server::SendFileToOther(SOCKET* sock, std::string filepath) {

    std::thread SendFileThread(SendFile, sock, filepath,this);
    SendFileThread.detach();
    return true;
}

//int recvAll(SOCKET* sock,char* buffer,int length) {
//    int totalReceived = 0;
//
//    while (totalReceived < length) {
//        //finish this
//    }
//
//
//}



void startReceiving(SOCKET* sock, char* message, bool* status, Server* MyServer) {

    while (true) {
        std::vector<char> receiveBuffer(RBuffAmount);

        int byteCount = recv((*sock), receiveBuffer.data(),sizeof(DataPacket), 0);
       

        if (byteCount == 0) {
            std::cout << "Maybe Client Disconnected!" << std::endl;
            MyServer->log("Maybe Client Disconnected!", 1);
            *status = false;
            //break;
            continue;
        }
        else if (byteCount < 0) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                printf("Receive error: %d\n", err);
                MyServer->log("Receive Error : " + err, 2);
                *status = false;
                break;
            }
        }
            // Ensure null termination
        receiveBuffer.resize(sizeof(DataPacket));
        try {

            DataPacket* DPack = DeserializeDataPacket(receiveBuffer.data(), byteCount);
            if (DPack == nullptr) {
                continue;
            }
            // Copy only if message is within bounds
            std::cout << "Dpack Datasize " << DPack->DataSize << std::endl;
            if (DPack->DataSize > 0) {

                switch (DPack->DataType) {

                case DT_Message:
                {


                    std::string receivedMessage(DPack->Data, DPack->Data + sizeof(DPack->Data));



                    memcpy(message, receivedMessage.c_str(), DPack->DataSize);
                    /*strcpy_s(message, 200, receiveBuffer);*/
                    message[DPack->DataSize] = '\0';

                    std::cout << "Received message: " << message << std::endl;


                    MyServer->log(std::string("Received Message : ") + message, 0);

                    *status = true;

                    break;
                }
                case DT_FileHeader:
                {

                    std::cout << "received header file!" << std::endl;
                    std::cout << "Sender name : " << DPack->SenderName << std::endl;
                    std::cout << "FileName : " << DPack->FileName << std::endl;
                    std::cout << "Data type : " << DPack->DataType << std::endl;
                    std::cout << "Data size : " << DPack->DataSize << std::endl;
                    std::cout << "Packet ID : " << DPack->PacketID << std::endl;
                    std::cout << "TotalNumberOfPackets : " << DPack->totalPackets << std::endl;


                    MyServer->ReceivingFileHeader = DPack;

                    MyServer->DataTransferred = 0;
                    MyServer->fileSize = DPack->DataSize;
                    MyServer->ReceivingFile = true;
                    break;
                }
                case DT_BinaryFile:
                {
                    if (MyServer->ReceivingFileHeader->DataType != -1) {

                        if (true) {


                            MyServer->DataTransferred += DPack->DataSize;
                            MyServer->sentProgress = static_cast<float>(MyServer->DataTransferred) / MyServer->fileSize;

                            std::cout << "Received data Packet : " << DPack->PacketID << std::endl;
                            std::cout << "data : " << DPack->Data << std::endl;
                            std::cout << "data size : " << DPack->DataSize << std::endl;

                            std::string fileName = MyServer->ReceivingFileHeader->FileName;
                            std::string folderPath = GetExecutableDirA() + "\\ReceivedFiles";
                            std::string fullPath = folderPath + "\\" + fileName;

                            // Create folder if it doesn't exist
                            struct stat info;
                            if (stat(folderPath.c_str(), &info) != 0) {
                                _mkdir(folderPath.c_str()); // Windows version
                            }

                            std::fstream outFile(fullPath, std::ios::in | std::ios::out | std::ios::binary);

                            // If file doesn't exist yet, create and open it
                            if (!outFile) {
                                outFile.open(fullPath, std::ios::out | std::ios::binary); // create
                                outFile.close();
                                outFile.open(fullPath, std::ios::in | std::ios::out | std::ios::binary);
                            }

                            if (!outFile) {
                                std::cerr << "Failed to open or create file: " << fullPath << std::endl;
                                break;
                            }

                            // Calculate offset and write packet data at correct position
                            std::streampos offset = static_cast<std::streampos>(DPack->PacketID) * (MyServer->MaxPacketDataSize);
                            outFile.seekp(offset);
                            outFile.write(DPack->Data, DPack->DataSize);
                            outFile.close();

                            //std::cout << "Packet " << DPack.PacketID << " written to " << fullPath << std::endl;


                            if (DPack->PacketID == MyServer->ReceivingFileHeader->totalPackets - 1) {
                                MyServer->ReceivingFile = false;
                                MyServer->sendingFileName = "";
                                float sentProgress = 0.0f;
                                int DataTransferred = 0;
                                int fileSize = 0;
                                FreeDataPack(MyServer->ReceivingFileHeader);
                            }

                        }
                    }
                    break;
                }


                }

            }
            else {
                std::cerr << "Message Corrupted." << std::endl;
                MyServer->log("Message Corrupted.", 1);
                *status = false;
            }
            if (DPack != MyServer->ReceivingFileHeader) {

                FreeDataPack(DPack);
            }

        }
        catch (const std::exception& e) {
            std::cerr << "Error during deserialization: " << e.what() << std::endl;
            MyServer->log("Deserialization Error", 2);
            *status = false;
        }

        Sleep(500);
    }

}




bool Server::ReceiveMessageFromOther(SOCKET* sock, char* message, bool* RecieveStatus) {

    std::thread receiveThread(startReceiving, sock, message,RecieveStatus,this);
    receiveThread.detach();
    return true;
}



