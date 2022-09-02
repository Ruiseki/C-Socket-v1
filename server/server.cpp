#include "server.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <winsock2.h>
#include <map>
#include "windows.h"
#include <sys/time.h>
#include <thread>

using namespace std;

Server::Server(int port_p) : bufferSize(128), dataBufferSize(50 * 1024), max_clients(2), clientVersion("a.0.15")
{
    client_sockets = new int[max_clients];
    client_names = new string[max_clients];
    buffer = 0;
    addrlen = sizeof(addr);
    port = port_p;
}

Server::Server() : Server(55000)
{
    
}

Server::~Server()
{
    delete[] client_sockets; client_sockets=0;
    delete[] client_names; client_names=0;
    delete[] buffer; buffer=0;
}

void Server::boot()
{
    this->wlog("\n\n### STARTING THE SERVER ###",2);
    WSADATA ws; // Initialiser le WSA, c'est un truc à windaube lié aux sockets mais je sais pas c'est kwa
    if(WSAStartup(MAKEWORD(2,2),&ws) < 0){
        //erreur : impossible d'initialiser le WSA
        wlog("WSA - MERDE!");
        throw new exception();
    }else wlog("WSA - OK!");

    addr.sin_addr.s_addr = INADDR_ANY; // indique que toutes les sources seront acceptées
    addr.sin_port = htons(port); // toujours penser à traduire le port en réseau
    addr.sin_family = AF_INET; // notre socket est TCP
    memset(&(addr.sin_zero),0,8); // je sais pas c'est kwa

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // déclaration du socket
    if(sockfd < 0){
        // erreur : impossible d'ouvrir le socket
        wlog("SOCKET - MERDE!");
        throw new exception();
    }else wlog("SOCKET - OK!");

    // int setsockopt()  option du socket
    int res = bind(sockfd,(sockaddr*)&addr,sizeof(addr));
    if(res < 0)
    {
        // erreur : impossible de se caller sur le port donné
        wlog("BIND - MERDE!");
        throw new exception();
    } else wlog("BIND - OK!");
}

void Server::run()
{
    int lis = listen(sockfd, max_clients); // max_clients = backlog. Nmbr de personne pouvant demander des trucs simultanément au serv
    if(lis < 0)
    {
        wlog("LISTEN - MERDE!");
        throw new exception();
    } else wlog("LISTEN - OK!");
   
    for(int i(0) ; i < max_clients ; i++) client_sockets[i]=0; // Initialise clients_socket

    while(true)
    {
        this->check();
        this->ops();
    }
}

void Server::check()
{
    /* FD_ZERO(&fwrite);
    FD_ZERO(&ferror);

    FD_SET(sockfd,&fread);
    FD_SET(sockfd,&ferror); */
    int sd, max_sd, activity;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&fread);

    FD_SET(sockfd, &fread);
    max_sd = sockfd;

    for(int i(0); i < max_clients; i++)
    {
        sd = client_sockets[i];

        if(sd > 0) FD_SET(sd, &fread);

        if(sd > max_sd) max_sd = sd;
    }

    /* for(map<int,string>::iterator UserSocket = Usernames.begin(); UserSocket!=Usernames.end();UserSocket++)
    {
        sd = (*UserSocket).first;

        if(sd > 0) FD_SET(sd,&fread);

        if(sd > max_sd) max_sd = sd;
    } */

    // int sel = select(max_sd, &fread, &fwrite, &ferror, &tv);

    activity = select(max_sd + 1, &fread, NULL, NULL, &tv);
    
    if(FD_ISSET(sockfd, &fread))
    {
        int new_socket = accept(sockfd, (sockaddr*)&addr, &addrlen);
        if(new_socket < 0)
        {
            // erreur : impossible d'accepter la connection
            wlog("ACCEPT - MERDE!");
            throw new exception();
        }else wlog("ACCEPT - OK!",2);

        boolean isServerFull = true;
        for(int i(0); i < max_clients ; i++)
        {
            if(client_sockets[i] == 0)
            {
                isServerFull = false;
                client_sockets[i] = new_socket;
                client_names[i] = "User";
                this->wlog("New client !");
                this->wlog("New client's position : "+to_string(i+1),2);
                break;
            }
        }

        if(isServerFull)
        {
            sendMsg(new_socket, "//serverfull");
            shutdown(new_socket, 2);
        }
    }
}

void Server::ops()
{
    int sd, valread;
    // operations on other socket
    for(int i(0); i < max_clients; i++)
    {
        sd = client_sockets[i];

        if(FD_ISSET(sd, &fread)) // check socket activity
        {
            buffer = new char[bufferSize];
            valread = recv(sd, buffer, bufferSize, 0);

            if(valread == 0)
            {
                if(client_names[i] != "UPDATER")
                {
                    cout << endl << client_names[i] << " s'est deconnecter." << endl << endl;
                    this->wlog("LOGOUT client #" + to_string(i+1) + " || name : " + client_names[i], 2);
                }
                else
                {
                    this->wlog("LOGOUT UPDATER");
                }
                shutdown(sd, 2); // closing connection
                client_sockets[i] = 0; // clean for reuse
                client_names[i] = "";
            }
            else if(valread > 0)
            {
                buffer[valread]='\0';
                string msg = buffer;
                this->wlog("Incoming : " + msg, 2);

                if(msg.substr(0, 10) == "//version=") // checking client version when asked
                {
                    this->wlog("Receiving client version info");
                    string checkingVersion;
                    checkingVersion = msg.substr(10, msg.size());
                    if(clientVersion == checkingVersion)
                    {
                        this->sendMsg(sd, "//check");
                        this->wlog("The client is up to date",2);
                    }
                    else
                    {
                        this->sendMsg(sd,"//needDl");
                        this->wlog("The client need to update his programme",2);
                        // this->uploadClientVersionUpToDate(sd);
                    }
                }

                else if(msg.substr(0, 7) == "//name=") // get the name of the concerned client
                {
                    this->wlog("Receiving client name info");
                    client_names[i] = msg.substr(7, msg.size());
                    if(client_names[i] != "UPDATER")
                    {
                        cout << endl << client_names[i] << " s'est connecter !" << endl << endl;
                        this->wlog("LOGINT client #" + to_string(i+1) + " || name : " + client_names[i], 2);
                    }
                    else this->wlog("LOGIN UPDATER");

                    this->sendMsg(sd, "//check");
                }

                else if(msg == "//test")
                {
                    
                }

                else if(msg == "//dl")
                {
                    this->wlog("A client want to test the download process");
                    
                    string path;
                    ifstream isExiste;
                    do
                    {
                        int val = recv(sd, buffer, bufferSize, 0); // get the file path
                        buffer[val] = '\0';
                        path = buffer;
                        isExiste.open(path.c_str());
                        
                        if(isExiste)
                        {
                            this->sendMsg(sd, "//ready");
                        }
                        else
                        {
                            this->sendMsg(sd, "//retry");
                            isExiste.close();
                        }

                    }while(!isExiste);

                    isExiste.close();

                    this->wlog("Waiting the client ...");
                    recv(sd, buffer, bufferSize, 0);
                    
                    delete[] buffer; buffer=0;
                    this->uploadFile(path.c_str(), sd);
                }

                else if(msg == "//testUpload")
                {
                    this->wlog("//testUpload received");
                    char buffer[bufferSize];
                    this->sendMsg(sd, "//ready");
                    buffer[recv(sd, buffer, bufferSize, 0)] = '\0';
                    string outputPath = buffer;
                    this->downloadFile(sd, outputPath);
                }

                else if(msg == "//updater")
                {
                    this->wlog("updater requesting download");
                    this->sendMsg(sd, "//handshake");

                    this->wlog("Uploading exe");
                    this->uploadFile("../clientSource/client.exe", sd);

                    this->wlog("Sending the correct version number : " + clientVersion);
                    this->sendMsg(sd, "123456789");
                }

                else if(msg.substr(0,2) == "//")
                {
                    
                }
                
                else
                {
                    cout << client_names[i] << " -> " << msg << endl;
                    this->wlog("Client #" + to_string(i+1) + " " + client_names[i] + " -> " + msg);
                }
            }
            else
            {
                // Client lost connection
                wlog("Client #" + to_string(i+1) + " " + client_names[i] + " unreachable. Terminating his connection.");
                cout << endl << client_names[i] << " a disparu !" << endl << endl;
                shutdown(sd, 2); // closing connection
                client_sockets[i] = 0; // clean for reuse
                client_names[i] = "";
            }
            
            delete[] buffer;
            buffer = 0;
        }
    }
}

void Server::wlog(string msg,int enter)
{   
    struct timeval time_now{};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
                  
    ofstream write("./log.txt", ios::app);
    write<<timeinfo->tm_hour<<':'<<timeinfo->tm_min<<':'<<timeinfo->tm_sec<<':'<<msecs_time%1000<<'\t';
    write<<msg;
    for(int i(0);i<enter;i++) write<<endl;
}

void Server::wlog(string msg)
{
    this->wlog(msg,1);
}

void Server::sendMsg(int socket, string msg, bool isLoged)
{
    send(socket, msg.c_str(), msg.size(), 0);
    if(isLoged) this->wlog("Message send : " + msg);
}

void Server::sendMsg(int socket, string msg)
{
    this->sendMsg(socket, msg, true);
}

void Server::downloadFile(int socket, string outputPath)
{
    int valread, streamSize,
    fileSize, totalPacket;

    ofstream output(outputPath.c_str(), ios::binary);
    char data[dataBufferSize], buf[bufferSize];

    this->sendMsg(socket, "//ready", false); // ready to download
    this->wlog("Waiting the //ready...");
    if(recv(socket, buf, bufferSize, 0) < 0)
    {   
        wlog("Failed receiving");
        throw new exception();
    }

    valread = recv(socket, buf, bufferSize, 0);
    buf[valread] = '\0';
    fileSize = stoi(string(buf));

    totalPacket = fileSize / dataBufferSize + 1;

    this->sendMsg(socket, "//next");

    this->wlog("Starting download...");
    do
    {
        valread = recv(socket, buf, bufferSize, 0); // streamSize reception
        buf[valread] = '\0';

        if(string(buf) == "//last")
        {
            this->sendMsg(socket, "//next", false); // ready for the next operation
            valread = recv(socket, buf, bufferSize, 0); // final streamSize reception
            buf[valread] = '\0';

            // this->wlog("nop");
            streamSize = stoi((string)buf); // operational streamSize
            this->sendMsg(socket, "//next", false); // ready for the next operation
            recv(socket, data, dataBufferSize, 0); // final buffer reception
            output.write(data, streamSize);
            this->sendMsg(socket, "//end", false);
            break;
        }
        // this->wlog("ok");
        streamSize = stoi(string(buf)); // operational streamSize
        this->sendMsg(socket, "//next", false); // ready for the next operation

        recv(socket, data, dataBufferSize, 0); // binary data reception
        output.write(data,streamSize); // writing binary data into the output

        this->sendMsg(socket, "//next", false); // ready for the next operation
    }while(true);

    this->wlog("Download completed !",2);
}

void Server::uploadFile(string inputPath, int targetSocket)
{
    ifstream input(inputPath.c_str(), ios::binary);
    char buf[bufferSize], data[dataBufferSize];

    // --------------------------------------------------
    // checking the file

    this->wlog("File : " + inputPath);
    if(!input)
    {
        this->wlog("Fatal error : cant read the file");
        throw new exception();
    }
    
    // --------------------------------------------------
    // sending the size of the file

    this->wlog("Sending the fileSize ...");
    input.seekg(0, ios::end);                               // place the cursor of the file at the end ...
    this->wlog("File size : " + to_string(input.tellg()));  // ... and watch his position
    this->sendMsg(targetSocket, to_string(input.tellg()));  // send the result to the client
    input.seekg(0, ios::beg);                               // replace the cursor at the begining of the file

    // --------------------------------------------------
    // final handshake

    this->wlog("Waiting the client...");
    recv(targetSocket, buf, bufferSize, 0);
    this->sendMsg(targetSocket, "//go");

    // --------------------------------------------------
    // sending binary

    this->wlog("Starting upload...");

    int packetCounter = 0;
    
    while(input.read(data, dataBufferSize))
    {
        packetCounter++;
        send(targetSocket, data, dataBufferSize, 0); // send the binary data
        recv(targetSocket, buf, bufferSize, 0); // Waiting the client
    }

    // last packet
    packetCounter++;
    send(targetSocket, data, dataBufferSize, 0); // send the binary data

    recv(targetSocket, buf, bufferSize, 0); // Waiting the client

    // --------------------------------------------------

    this->wlog("Number of packet sended : " + to_string(packetCounter));
    this->wlog("Upload completed !", 2);
}