#include "Server.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <3ds.h>

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>

#include <malloc.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

Server::Server(int port){

    this->port = port;

}

void Server::start(){

    int MAX_CONNECTIONS = 1;

    u32* SOC_BUFFER = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if(SOC_BUFFER == NULL){
        std::cerr<<"Error in memalign allocation\n";
        return ;
    }

    int socInitreturn = socInit(SOC_BUFFER, SOC_BUFFERSIZE);
    if(socInitreturn != 0){
        std::cerr<<"Error initializing soc:u service, socinit: "<<(unsigned int)socInitreturn<<"\n";
        return ;
    }

    /* 
    =================
     Creating socket
    =================
    */
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(serverSocket == -1){
        std::cerr<<"Error creating socket.\n";
        return ;
    }

    /*
    =====================
     Configuring address
    =====================
    */
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;


    if(bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        std::cerr<<"Error turning on server.\n";
        return ;
    }

    if(listen(serverSocket, MAX_CONNECTIONS) == -1){
        std::cerr<<"Error putting server on listening mode\n";
        return ;
    }
    u32 localIP = gethostid();
    u8* localIPBytes = (u8*)&localIP;
    printf("Server running at: %d.%d.%d.%d:%d\n", localIPBytes[0], localIPBytes[1], localIPBytes[2], localIPBytes[3], port);

    this->serverSocket = serverSocket;

}

void Server::receiveClientRequest(){
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);

        if(clientSocket == -1){
            std::cerr<<"Error obtaining client socket\n";
            return ;
        }

        handleRequest(clientSocket);

        close(clientSocket);
}


void Server::handleRequest(int clientSocket){

    /*
        =====================================
         Getting bytes received from request
        =====================================
    */
    char buffer[4096];
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if(bytesReceived < 0){
        std::cerr<<"Error with request.\n";
        return ;
    }
    std::string request(buffer, bytesReceived);

    /*
        ========================
         Getting request method
        ========================
    */
    std::string method, path;
    std::istringstream iss(request);
    iss >> method >> path;

    if(routes.count(path) && routes[path].count(method)){
        routes[path][method](request, clientSocket);
    }
    else{
        std::string response = "Not found!";

        sendResponse(clientSocket, response);
    }
}

void Server::addRoute(const std::string& path, const std::string& method, std::function<void(std::string, int)> handler){
    routes[path][method] = handler;
}

void Server::sendResponse(int clientSocket, const std::string& response){
    send(clientSocket, response.c_str(), response.size(), 0);
}

std::string Server::getPostData(std::string request){
    size_t pos = request.find("\r\n\r\n");
    if(pos != std::string::npos){
        std::string postData = request.substr(pos + 4);
        return postData;
    }
    return nullptr;
}

void Server::stop(){
    closesocket(serverSocket);
}