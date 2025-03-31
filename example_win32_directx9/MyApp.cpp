#include "MyApp.h"
#include "imgui.h"



bool isValidIP(const char* ip) {
    std::stringstream ss(ip);
    std::string segment;
    int value;
    int segmentCount = 0;

    while (std::getline(ss, segment, '.')) {
        try {
            // Check if the segment can be converted to an integer
            value = std::stoi(segment);
            // Validate the range
            if (value < 0 || value > 255) return false;
            segmentCount++;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    return segmentCount == 4; // Should have exactly 4 segments
}




namespace MyApp {

    //flags for wrong ip entry-text-effect 
    static bool showError = false;
    static int ShowErrorShake = 0;


    //Cond flags for client connecting server 
    static bool ConnectionStatus = false;
    static bool ShowConnectionStatus = false;
    static int ShowConnectionStatusShake = 0;
    static float LoadingPB = 0.0f;
    

    //Messages handling:-
    static char MessageBuffer[10][100];
    static int MessageFrom[10];
    static int MessageCounter = 0;
    static int RecieveMessagePadding[10];
    static bool ServerReceiveThread=false;
    static bool RecieveStatus = false;
    
    //Start Server handling:-
    static bool StartServer=false;
    

   

    void ClearSendMessageBuffer() {
        for (int i = 0; i < 10; i++) {
           strcpy(MessageBuffer[i],"");
        }
    }


    void MessageBoxWin(bool* IPwin,Server* myServer,char* Sendbuffer,char* RecieveBuffer) {

        

        if (ImGui::Begin("##MessageWin")) {
            
            
            if (myServer->ConnectionStatus) {
                if (myServer->MSGsock == INVALID_SOCKET) {
                    if (myServer->connectionType == 1) {
                        std::cout << "mysocket status : " << (myServer->mySocket == INVALID_SOCKET) << std::endl;
                        myServer->MSGsock = myServer->mySocket;
                    }
                    else if (myServer->connectionType == 2) {

                        std::cout << " status : " << (myServer->acceptSocket == INVALID_SOCKET) << std::endl;

                        myServer->MSGsock = myServer->acceptSocket;
                    }
                    else {
                        std::cout << "socket type error check socket validity" << std::endl;
                    }
                }

                ImGui::SetCursorPos(ImVec2(10,480));
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),"Enter your message : ");

                ImGui::SetCursorPos(ImVec2(10, 500));
                ImGui::InputText("##Message", Sendbuffer,200);

                ImGui::SetCursorPos(ImVec2(850, 500));
                if ((ImGui::Button("Send") || ImGui::IsKeyPressed(ImGuiKey_Enter)) && Sendbuffer[0]!='\0') {
                    ImGui::SetNextItemWidth(400);

                    
                    myServer->SendMessageToOther(&myServer->MSGsock,Sendbuffer);

                    if (MessageCounter == 9) {
                        ClearSendMessageBuffer();
                        MessageCounter = 0;

                    }
                    strcpy(MessageBuffer[MessageCounter], Sendbuffer);
                    MessageFrom[MessageCounter] = 0;
                    MessageCounter++;
                    strcpy(Sendbuffer, "");
                    
                }
                if (!ServerReceiveThread) {
                    myServer->ReceiveMessageFromOther(&myServer->MSGsock, RecieveBuffer,&RecieveStatus);
                    ServerReceiveThread = true;
                }
                if ( ServerReceiveThread && RecieveStatus &&(strcmp(RecieveBuffer,"")>0) ) {
                    //

                    std::cout << "debug for message recieved! in myapp" << std::endl;
                    std::cout << RecieveBuffer << std::endl;
                    RecieveStatus = false;
                    RecieveMessagePadding[MessageCounter] = strlen(RecieveBuffer) * 7;
                    if (MessageCounter == 9) {
                        ClearSendMessageBuffer();
                        MessageCounter = 0;

                    }
                    strcpy(MessageBuffer[MessageCounter], RecieveBuffer);
                    MessageFrom[MessageCounter] = 1;
                    MessageCounter++;
                    strcpy(RecieveBuffer, "");

                }


                int MessagePosY = 30;
                int RecieveMessagePosX = 1100;
                for (int i = 0; i < MessageCounter; i++) {

                    
                    if (MessageFrom[i] == 0) {
                        ImGui::SetCursorPos(ImVec2(30, MessagePosY));
                        ImGui::TextColored(ImVec4(0.0f,1.0f,0.0f,1.0f), "You : %s", MessageBuffer[i]);
                        MessagePosY += 30;
                    }
                    else if(MessageFrom[i] == 1) {
                        ImGui::SetCursorPos(ImVec2(RecieveMessagePosX-RecieveMessagePadding[i], MessagePosY));
                        ImGui::TextColored(ImVec4(0.419f, 0.8117f, 0.878f, 1.0f), "%s : %s",myServer->ConnectedDeviceIP.c_str(), MessageBuffer[i]);
                        MessagePosY += 30;

                    }
                }

            }
        }

        ImGui::End();






        
        

            
    }

    void TopBar(bool* IpWin,Server* myServer) {

        //Code for Docking :-

        static bool opt_fullscreen = true;
        static bool opt_padding = false;


        


        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

      
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", NULL, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        
        ImGui::End();


        // App code:-
        ImGui::Begin("##Test");

        if(!StartServer){


            ImGui::SetCursorPos(ImVec2(30, 50));
            if (ImGui::Button("Connect To device")) {
            

                *IpWin = true;
            }

            ImGui::SetCursorPos(ImVec2(170, 50));
            if (ImGui::Button("Start a Server")) {
           
                StartServer = true;
                myServer->StartServer();
                myServer->Listen();
                
                //check here
                myServer->StartAcceptingConnections();
                  
            }
            
        }
        else {

            if (myServer->isClientConnected) {
                myServer->connectionType = 2;
                myServer->ConnectionStatus = true;
                myServer->ConnectedDeviceIP = myServer->ClientIP;
                //std::cout << "Connected successfully ";
            }

            if (myServer->ConnectionStatus) {

                ImGui::SetCursorPos(ImVec2(10, 90));
                ImGui::Text("Connected Device IP : ");
                ImGui::SetCursorPos(ImVec2(10, 110));

                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), myServer->ConnectedDeviceIP.c_str());

                //myServer->SendMessageToOther(&myServer->mySocket);


            }
            else {
                ImGui::SetCursorPos(ImVec2(50,50));
                ImGui::Text("Waiting for Connections");

            }
        }
        
        

        



       

        //ImGui::ShowDemoWindow();
        ImGui::End();
      
    }
    void GetIpWindow(char* IPbuff,bool* IPWin,Server* MyServer) {

        


        ImGui::SetNextWindowSize(ImVec2(300, 250));
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.085f, 0.085f, 0.085f, 0.7f)); // Dark Blue


        ImGui::Begin("GetIp",NULL,ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(ImVec2(30, 100));
        ImGui::Text("Enter IP Address of the host device");


        ImGui::SetCursorPos(ImVec2(50, 120));
        ImGui::InputText("##Ip",IPbuff,30);

        int xOffset = (rand() % 2 == 0) ? -(rand() % 6 + 5) : (rand() % 6 + 5);  // X shake range: -10 to +10
        int yOffset = (rand()%2 == 0) ? -(rand() % 3 + 3) : (rand() % 3 + 3);  // Y shake range: -5 to +5

        if (ShowErrorShake < 30) {
            ImGui::SetCursorPos(ImVec2(90+xOffset, 185+yOffset));
            ShowErrorShake++;
        }
        else {
            ImGui::SetCursorPos(ImVec2(90, 185));
        }

        if (showError) {

            ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,100.0f),"Enter a valid IP");
        }

        ImGui::SetCursorPos(ImVec2(90, 150));
        if (ImGui::Button("Submit") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if (isValidIP(IPbuff)) {


                std::string  IPstr(IPbuff);
                //issue with creating thread is i will create multiple threads and need to join it after finishing execution
                //needs a lot of flag variables and code becomes muddy
                /*std::thread ClientConnectThread(&Server::ConnectServer, &MyServer, IPstr,&ConnectionStatus);*/



                ConnectionStatus=(MyServer->ConnectServer(IPstr)); //using this causes loading gui to just be a gimmic and the
                                                                        //code becomes blocking


                ShowConnectionStatus = true;
                showError = false;
                LoadingPB = 0.0f;
                ShowConnectionStatusShake = 0;

                





                //*IPWin = false;
                
            }
            else {
                showError = true;
                ShowErrorShake = 0;
                
            }


           
            
           

        }

        ImGui::SetCursorPos(ImVec2(160, 150));
        if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            *IPWin = false;
            showError = false;
            ShowErrorShake = 0;
            ShowConnectionStatus = false;
        }

        
        if (ShowConnectionStatus) {

            if(LoadingPB < 1.0f){

                ImGui::SetCursorPos(ImVec2(55, 185));
                ImGui::ProgressBar(LoadingPB, ImVec2(180.0f, 15.0f), "Connecting");
                LoadingPB += 0.005f;
                
            }
            else {
                



                if (ConnectionStatus) {

                    ImGui::SetCursorPos(ImVec2(75, 185));
                    ImGui::TextColored(ImVec4(0.0f,1.0f,0.0f,1.0f),"Connected Successfully");
                    MyServer->ConnectedDeviceIP = IPbuff;
                    MyServer->ConnectionStatus = true;
                    MyServer->connectionType = 1;
                    *IPWin = false;
                }
                else {

                    int xOffset = (rand() % 2 == 0) ? -(rand() % 6 + 5) : (rand() % 6 + 5);  // X shake range: -10 to +10
                    int yOffset = (rand() % 2 == 0) ? -(rand() % 3 + 3) : (rand() % 3 + 3);  // Y shake range: -5 to +5
                    if (ShowConnectionStatusShake < 30) {
                        ImGui::SetCursorPos(ImVec2(90 + xOffset, 185 + yOffset));
                        ShowConnectionStatusShake++;
                    }
                    else {
                        ImGui::SetCursorPos(ImVec2(90, 185));
                    }
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed To Connect");

                }


            }

        }
        
        ImGui::End();
    }


    void Console(Server* MyServer) {
        if (ImGui::Begin("##Console")) {

            if (!MyServer->Applog.empty()) {
                if (ImGui::Button("Clear")) {
                    MyServer->Applog.clear();
                }
            }

            ImGui::BeginChild("Console",ImVec2(0,0),true,ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& log : MyServer->Applog) {
                // Set color based on log level
                if (log.level == 0)
                    ImGui::TextColored(ImVec4(1, 1, 1, 1), "[INFO] %s", log.message.c_str());
                else if (log.level == 1)
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "[WARNING] %s", log.message.c_str());
                else
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "[ERROR] %s", log.message.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f); 
            }




            ImGui::EndChild();
            ImGui::End();
        }
    }


}







