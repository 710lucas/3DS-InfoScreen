#include "Classes/Server.hpp"
#include <iostream>

#include <3ds.h>

int main(){
    Server server(8080);

    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    server.addRoute("/ping", "GET", [&server](std::string request, int clientSocket){
        server.sendResponse(clientSocket, "Pong");
    });

    server.addRoute("/STOP", "GET", [&server](std::string request, int clientSocket){
        server.stop();
    });

    server.addRoute("/", "POST", [&server](std::string request, int clientSocket){

        std::string data = server.getPostData(request);
        std::cout << "\033[2J\033[1;1H";
        std::cout<<data<<"\n";
        server.sendResponse(clientSocket, data);

    });

    server.start();

    while(aptMainLoop()){
        gspWaitForVBlank();



        hidScanInput();

        u32 kDown = hidKeysDown();
        if(kDown & KEY_START){
            server.stop();
            break;
        }
        server.receiveClientRequest();


    }

    gfxExit();
    return 0;
}