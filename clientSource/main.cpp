// Ne pas oublier d'inclurer "-lwsock32" après "client.exe" dans la commande de compilation.

#include <iostream>
#include <string>
#include <winsock2.h>
#include <fstream>
#include "windows.h"
#include "client.hpp"
using namespace std;

int main(int argc, char *argv[])
{
    string serverAddr, port;
    
    if(argc > 1)
    {
        if(argc == 1) port = argv[1];
        else
        {
            port = argv[1];
            serverAddr = argv[2];
        }
    }
    else
    {
        serverAddr = "127.0.0.1";
        port = "55000";
    }

    Client clientapp(stoi(port), serverAddr);

    int selection(1);
    string command;
    while(selection!=0)
    {
        system("cls");
        cout << "Current pseudo : " << clientapp.getPseudo() << endl;
        cout << "Current port : " << clientapp.getPort() << endl;
        cout << "Server IP : " << clientapp.getIP() << endl;
        cout << "1: Change pseudo" << endl <<
        "2: Change port" << endl <<
        "3: Change IP" << endl <<
        "4: connect to the server" << endl <<
        "8: Manual update" << endl <<
        "0: shutdown client" << endl;
        cin >> selection;

        switch(selection)
        {
            case 0:
                break;

            case 1:
                clientapp.changePseudo();
                break;

            case 2:
            {
                int newPort;
                system("cls");
                cout << "New port : ";
                cin >> newPort;
                clientapp.setPort(newPort);
                break;
            }
            
            case 3:
            {
                string newIP;
                system("cls");
                cout << "New server IP : ";
                cin >> newIP;
                clientapp.setIP(newIP);
                break;
            }

            case 4:
                clientapp.run();
                break;
                
            case 8:
                command = "updater.exe " + port + " " + serverAddr;
                system(command.c_str());
                return 0;
                break;

            default:
                system("cls");
                break;
        }
    }
    system("cls");
    return 0;
}