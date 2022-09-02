#include <iostream>
#include <string>
#include <fstream>
#include <winsock2.h>
#include "windows.h"
#include <sys/time.h>

using namespace std;

void wlog(string text)
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
    write<<text<<endl;
}

void sendMsg(int master_socket, string msg)
{
    int success = send(master_socket,msg.c_str(),msg.size(),0);
    if(success < 0)
    {
        wlog("[UPDATER] Failed to send :\""+msg+"\"");
        throw new exception();
    }
}

void downloadFile(int master_socket, string outputPath)
{
    int valread, fileSize, totalPacket, packetId(0);

    int bufferSize(128), dataBufferSize(50 * 1024);

    ofstream output(outputPath.c_str(), ios::binary);
    char data[dataBufferSize], buf[bufferSize];

    valread = recv(master_socket, buf, bufferSize, 0); // recevoir la taille du fichier
    buf[valread] = '\0';
    fileSize = stoi(string(buf));

    totalPacket = fileSize / dataBufferSize;
    int lastBufferSize = dataBufferSize;
    if((totalPacket * dataBufferSize < fileSize)) 
    {
        lastBufferSize = fileSize - (totalPacket * dataBufferSize);
        totalPacket += 1;
    }

    sendMsg(master_socket, "//next");
    recv(master_socket, buf, bufferSize, 0);

    wlog("[UPDATER] Starting download...");
    
    while(packetId < totalPacket - 1)
    {
        recv(master_socket, data, dataBufferSize, 0); // binary data reception
        output.write(data, dataBufferSize); // writing binary data into the output

        sendMsg(master_socket, "//next"); // ready for the next operation
        packetId++; // packet number
    }

    recv(master_socket, data, lastBufferSize, 0); // binary data reception
    output.write(data, lastBufferSize); // writing binary data into the output

    sendMsg(master_socket, "//final"); // ready for the next operation
    // end

    wlog("[UPDATER] Download completed !");
}

int main(int argc, char *argv[])
{
    int master_socket, valread, port, addrlen;
    string serverAddr;
    WSADATA ws;
    int const bufferSize(128);
    char* buffer;
    sockaddr_in addr;

    port = 55000;
    serverAddr = "93.16.2.231";

    if(argc > 1)
    {
        if(argc == 2) port = stoi(argv[1]);
        else
        {
            port = stoi(argv[1]);
            serverAddr = argv[2];
        }
    }

    system("cls");

    string clientName("User");
    ifstream data("./data.txt");
    if(data) data >> clientName;
    data.seekg(0, ios::beg);

    addrlen = sizeof(addr);

    cout << "Shuting down the old client" << endl;
    system("TASKKILL /F /IM client.exe");
    Sleep(500);
    cout << "Deleting the old client" << endl;
    system("DEL client.exe");
    Sleep(500);

    if(WSAStartup(MAKEWORD(2,2),&ws) < 0) {cout<<"Error : WSADATA" << endl; return -1;}

    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    memset(&(addr.sin_zero),0,8);
    addr.sin_addr.s_addr = inet_addr(serverAddr.c_str());

    if(addr.sin_addr.s_addr <= 0) {cout<<"Error : server adress" << endl; return -1;}

    master_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(master_socket < 0) {cout<<"Error : master_socket declaration" << endl; return -1;}

    while(connect(master_socket, (sockaddr*)&addr, addrlen) < 0)
    {
        cout << "Connection to the server impossible. Retry in 5 seconds" << endl;
        Sleep(5000);
    }

    // ------------------------------------------------------------
    // Start updating

    buffer = new char[bufferSize];

    cout << "Waiting a response ..." << endl;

    wlog("Updater -> //name=UPDATER");
    send(master_socket, "//name=UPDATER", strlen("//name=UPDATER"), 0);

    recv(master_socket, buffer, bufferSize, 0);

    wlog("Updater -> //updater");
    send(master_socket, "//updater", strlen("//updater"), 0);

    recv(master_socket, buffer, bufferSize, 0);

    cout << "Downloading .exe" << endl;
    wlog("[UPDATER] Downloading .exe");
    downloadFile(master_socket, "./client.exe");
    cout << "Done" << endl << endl;


    wlog("[UPDATER] Waiting the new version number ...");
    // -------------------------------------------------------------------------------------
    // WTTTFFFFF
    cout << "valread foireux = " << recv(master_socket, buffer, bufferSize, 0) << endl;
    buffer[valread] = '\0';
    cout << "buffer foireux = " << (string)buffer << endl;
    wlog("[UPDATER] lastest version : " + (string)buffer);
    // -------------------------------------------------------------------------------------

    ifstream readData("./data.txt");
    string text;

    if(!readData) ofstream("./data.txt") << clientName << endl << string(buffer);
    else 
    {
        getline(readData, text);
        ofstream("./data.txt") << text << endl << string(buffer);
    }

    delete [] buffer; buffer = 0;

    shutdown(master_socket, 2);

    return 0;

    string command;
    command = "client.exe " + to_string(port) + " " + serverAddr;
    
    system(command.c_str());
    
    return 0;
}