#pragma once

#include <string>
#include <map>
#include <functional>

class Server{

private:
    int serverSocket;
    int port;
    std::map<std::string, std::map<std::string, std::function<void(std::string, int)>>> routes;

    void handleRequest(int clientSocket);

public:
    Server(int port);
    void start();
    void addRoute(const std::string& path, const std::string& method, std::function<void(std::string, int)> handler);
    void sendResponse(int clientSocket, const std::string& response);

    std::string getPostData(std::string request);

    void receiveClientRequest();

    void stop();


};