// Ne pas oublier d'inclurer "-lwsock32" après "server.exe" dans la commande de compilation.

#include "server.hpp"
#include "windows.h" 
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    int port(55000);
    if(argc > 1)
    {
        port = stoi(argv[1]);
    }
    system("cls");
    cout << "Demarrage du serveur sur le port " << port << endl;
    Server server(port);

    server.boot();
    server.run();

    return 0;
}

// Vitesse avant : 3,5 Mo/s