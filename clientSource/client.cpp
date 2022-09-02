#include "client.hpp"
#include <winsock2.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include "windows.h"
#include <regex>
using namespace std;

Client::Client(int port_p, string serverIP) : bufferSize(128), dataBufferSize(50 * 1024)
{
    _serverIP = serverIP;

    this->wlog("\n\n ##STARTING THE CLIENT##", 2);
    buffer = 0;
    addrlen = sizeof(addr);
    port = port_p;
    clientName = "User";
    this->wlog("Port set at : " + to_string(port));
    this->wlog("Server IP set at : " + _serverIP);
    this->wlog("Reading data.txt...");
    ifstream dataRead("./data.txt");

    string text;
    if(dataRead)
    {
        getline(dataRead, text);
        this->wlog("File found. Pseudo : " + text);
        clientName = text;
        getline(dataRead, text);
        this->wlog("Version : " + text);
        clientVersion = text;
    }
    else
    {
        this->wlog("File not found. Creating a new one...");
        clientVersion = "0.0.0";
        ofstream("./data.txt") << clientName << endl << clientVersion;
        this->wlog("Done. Pseudo : " + clientName);
    }
}

Client::Client() : Client(55000, "93.16.2.231")
{
    
}

Client::~Client()
{
    delete[] buffer; buffer = 0;
}

void Client::run()
{
    system("cls");
    cin.ignore();
    this->socketBuild();
    string message;
    cout << sockfd << '\n';
    message = this->serverConnect();
    // this->coms();

    while(message != "//close")
    {
        getline(cin, message);
        if(message != "//close")
        {
            this->sendMsg(message);
        }

        if(message == "//dl")
        {
            system("cls");
            this->testDownload();
            system("cls");
        }
    }
    shutdown(sockfd, 2);    // shutdown : termine une connection.
                            // dernier parametre :
                            // 0 = mettre fin aux opération de récéptions
                            // 1 = mettre fin aux opération d'envoies
                            // 2 = mettre fin aux deu
}

void Client::socketBuild()
{
    WSADATA ws;//Initialiser le WSA, je sais pas c'est kwa
    if(WSAStartup(MAKEWORD(2,2),&ws) < 0)
    {
        //erreur
        //impossible d'initialiser le WSA
        this->wlog("WSA - MERDE!");
        throw new exception();
    }this->wlog("WSA - OK!");

    
    addr.sin_port = htons(port); // toujours penser à traduire le port en réseau
    addr.sin_family = AF_INET; // notre socket est TCP
    memset(&(addr.sin_zero),0,8);
    addr.sin_addr.s_addr = inet_addr(_serverIP.c_str()); // Indique l'adresse du serveur

    if(addr.sin_addr.s_addr <= 0)
    {
        // erreur
        // impossible d'ouvrir le socket
        this->wlog("ADRESSE - MERDE!");
        throw new exception();
    }this->wlog("ADRESSE - OK!");

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // déclaration du socket

    if(sockfd < 0)
    {
        // erreur
        // impossible d'ouvrir le socket
        this->wlog("SOCKET - MERDE!");
        throw new exception();
    }else this->wlog("SOCKET - OK!");
}

string Client::serverConnect()
{
    int valread;
    string text;

    if(connect(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        this->wlog("CONNECT - MERDE!");
        throw new exception();
    }this->wlog("CONNECT - OK!");

    this->sendMsg("//name=" + clientName);

    buffer = new char[bufferSize];
    recv(sockfd, buffer, bufferSize, 0);
    if(buffer == "//serverfull")
    {
        shutdown(sockfd, 2);
        return "//close";
    }
    this->wlog("The server get the client's name");
    delete [] buffer;

    buffer = new char[bufferSize];
    
    this->sendMsg("//version=" + clientVersion);
    
    valread = recv(sockfd, buffer, bufferSize, 0);
    buffer[valread] = '\0';
    text = buffer;
    if(text == "//cheak")
    {
        this->wlog("Server -> //check");
        this->wlog("Client Version is good");
    }
    else if(text == "//needDl")
    {
        this->wlog("Server -> //needDl");
        this->wlog("Need to download the new version");
        // this->downloadClientVersionUpToDate();
        string command("updater.exe ");
        command += port;
        command += " ";
        command += _serverIP;
        system(command.c_str());
        shutdown(sockfd, 2);
        exit(0);
    }

    delete [] buffer; buffer = 0;

    return "";
}

void Client::coms()
{

}

void Client::wlog(string msg, int enter)
{
    struct timeval time_now{};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    ofstream write("./log.txt", ios::app);
    write << timeinfo->tm_hour << ':' << timeinfo->tm_min << ':' << timeinfo->tm_sec << ':' << msecs_time%1000 << '\t';
    write << msg;
    for(int i(0); i < enter; i++) write << endl;
}

void Client::wlog(string msg)
{
    this->wlog(msg, 1);
}

void Client::sendMsg(string msg, bool isLoged)
{
    send(sockfd, msg.c_str(), msg.size(), 0);
    if(isLoged) this->wlog("Message send : " + msg);
}

void Client::sendMsg(string msg)
{
    this->sendMsg(msg, true);
}

string Client::getPseudo()
{
    return clientName;
}

int Client::getPort()
{
    return port;
}

string Client::getIP()
{
    return _serverIP;
}

void Client::changePseudo()
{
    system("cls");
    cin.ignore();
    cout << "New pseudo : ";
    getline(cin, clientName);
    ofstream("./data.txt") << clientName << endl << clientVersion;
}

void Client::setPort(int newPort)
{
    port = newPort;
}

void Client::setIP(string newIP)
{
    _serverIP = newIP;
}

void Client::downloadClientVersionUpToDate()
{
    string newerVersionPath("./newClient.exe"),
    updateBat("./updatingclient.bat"),
    text;
    system("cls");

    this->wlog("Starting download \"newClient.exe\"...");
    cout<<"Starting download \"newClient.exe\"..."<<endl;
    this->downloadFile(newerVersionPath);                   // downloading the new version
    this->wlog("Download completed !",2);
    cout<<"Download completed !"<<endl;

    this->wlog("Starting download \"updatingclient.bat\"...");
    cout<<"Starting download \"updatingclient.bat\"..."<<endl;
    this->downloadFile(updateBat);                          // downloading the bat file for "installation"
    this->wlog("Download completed !",2);
    cout<<"Download completed !"<<endl;

    this->wlog("Changing the version number");              

    buffer = new char[bufferSize];
    int valread;

    valread = recv(sockfd,buffer,bufferSize,0);
    buffer[valread] = '\0';
    text = buffer;

    this->wlog("New version : "+text);
    ofstream("./data.txt") << clientName << endl << text ;

    delete[] buffer;
    buffer=0;

    shutdown(sockfd,2);
    system("start updatingclient.bat");
}

void Client::downloadFile(string outputPath)
{
    int valread, streamSize, fileSize, totalPacket, packetId(0), progression, progressionOld(-1);
    ofstream output(outputPath.c_str(), ios::binary);
    char data[dataBufferSize], buf[bufferSize];
    // progressionOld : old progression. We are going to update the display every 1% for optimisation.

    // --------------------------------------------------
    // getting the file size

    this->wlog("Waiting the fileSize...");
    buf[recv(sockfd, buf, bufferSize, 0)] = '\0';
    this->wlog((string)buf);
    fileSize = stoi(string(buf));

    // --------------------------------------------------
    // calculating the number of packet and the size of the last

    totalPacket = fileSize / dataBufferSize;
    int lastBufferSize = dataBufferSize;
    if((totalPacket * dataBufferSize < fileSize)) 
    {
        lastBufferSize = fileSize - (totalPacket * dataBufferSize);
        totalPacket += 1;
    }

    this->wlog("Number of packets : " + to_string(totalPacket));
    this->wlog("Last buffer size : " + to_string(lastBufferSize));

    // --------------------------------------------------
    // finale handshake

    this->sendMsg("//next");
    recv(sockfd, buf, bufferSize, 0);
    this->wlog("Final handshake - OK");

    // --------------------------------------------------
    // receiving binary

    this->wlog("Starting download...");
    
    while(packetId < totalPacket - 1)
    {
        // --------------------------------------------------
        // progress bar

        progression = packetId * 100 / totalPacket;

        if(progression > progressionOld)
        {
            system("cls");
            cout << "Downloading..." << endl << "[";
            int x;
            x = progression / 10;

            for(int i(0); i < x; i++) cout << "*";
            for(x; x < 10; x++) cout << " ";

            cout << "] " << progression << "%" << endl;
            progressionOld = progression;
        }

        // --------------------------------------------------

        recv(sockfd, data, dataBufferSize, 0); // binary data reception
        output.write(data, dataBufferSize); // writing binary data into the output


        this->sendMsg("//next",false); // ready for the next operation
        packetId++; // packet number
    }
    
    // last packet
    recv(sockfd, data, lastBufferSize, 0); // binary data reception
    output.write(data, lastBufferSize); // writing binary data into the output

    // --------------------------------------------------
    this->sendMsg("//final", false); // ready for the next operation
    this->wlog("Download completed !", 2);
}

void Client::uploadFile(string inputPath)
{
    this->wlog("File : " + inputPath);
    this->wlog("client socket : " + to_string(sockfd));
    ifstream input(inputPath.c_str(), ios::binary);
    char buf[bufferSize], data[dataBufferSize];
    int valread;
    
    if(!input)
    {
        this->wlog("Fatal error : cant read the file");
        throw new exception();
    }

    this->wlog("Waiting the //ready...");
    if(recv(sockfd, buf, bufferSize, 0) < 0)
    {
        this->wlog("Failed receiving");
        throw new exception();
    }
    
    this->wlog("//ready recieved");
    
    this->sendMsg("//ready");

    input.seekg(0, ios::end);
    this->wlog("File size : " + to_string(input.tellg()));
    this->sendMsg(to_string(input.tellg()));
    input.seekg(0, ios::beg);

    recv(sockfd, buf, bufferSize, 0);

    this->wlog("Starting upload...");

    while(input.read(data, dataBufferSize))
    {
        this->sendMsg(to_string(input.gcount()),false); // send streamSize
        recv(sockfd, buf, bufferSize, 0); // Waiting the client

        send(sockfd, data ,dataBufferSize, 0); // send the binary data
        recv(sockfd, buf, bufferSize, 0); // Waiting the client
    }
    this->sendMsg("//last",false); // Final paquet
    recv(sockfd, buf, bufferSize,0); // Waiting the client

    this->sendMsg(to_string(input.gcount()),false); // send streamSize
    recv(sockfd, buf, bufferSize, 0); // Waiting the client

    send(sockfd, data, dataBufferSize, 0); // send the binary data
    recv(sockfd, buf, bufferSize, 0); // Waiting the client

    this->wlog("Upload completed !",2);
}

void Client::testDownload()
{
    string path, userInput;
    ifstream isExist;

    buffer = new char[bufferSize];

    do
    {
        cout << "File to client : ";
        getline(cin, userInput);
        this->wlog("User input = " + userInput);
        path = "";
        for(char letter : userInput)
        {
            if(letter == '\\') path += '/';
            else if(letter != '"') path += letter;
        }
        this->wlog("Converted input : " + path);
        this->sendMsg(path);

        buffer[recv(sockfd, buffer, bufferSize, 0)] = '\0';//check for ready or retry
        this->wlog("Server -> " + (string)buffer);

    }while((string)buffer == "//retry");

    this->sendMsg("Make handshake", true);

    this->downloadFile("./file." + path.substr(path.size()-3, path.size()));
    
    delete [] buffer; buffer = 0;
}

void Client::testUpload()
{
    string inputPath;
    ifstream isExiste;
    do
    {
        system("cls");
        cout << "File : ";
        cin >> inputPath;
        isExiste.open(inputPath.c_str());
        if(!isExiste)
        {
            isExiste.close();
        }

    }while(!isExiste);

    isExiste.close();

    this->socketBuild();
    this->serverConnect();
    Sleep(1000);

    char buf[bufferSize];
    this->sendMsg("//testUpload");
    recv(sockfd, buf, bufferSize, 0);
    this->sendMsg("./file."+inputPath.substr(inputPath.size()-3,inputPath.size()));
    
    this->uploadFile(inputPath);
}