#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <string>

class Client
{
    public:
        Client(int port_p, std::string serverIP);
        Client();
        ~Client();
        void run();
        std::string getPseudo();
        int getPort();
        std::string getIP();
        void changePseudo();
        void setIP(std::string newIP);
        void setPort(int newPort);
        void testDownload();
        void testUpload();

    private:
        void socketBuild();
        std::string serverConnect();
        void wlog(std::string msg,int enter);
        void wlog(std::string msg);
        void sendMsg(std::string msg, bool isLoged);
        void sendMsg(std::string msg);
        void coms();
        void downloadClientVersionUpToDate();
        void downloadFile(std::string outputPath);
        void uploadFile(std::string inputPath);

        int sockfd, addrlen, port;
        std::string clientName, clientVersion;
        char* buffer;
        int const bufferSize, dataBufferSize;
        std::string _serverIP;
        std::string* clientInfo;
        sockaddr_in addr;
        std::string message;
};

#endif