#include <iostream>
#include <winsock2.h>
#include "server.h"
void Server::Hello(){
    std::cout << "Hello" << std::endl;
}
void Server::Connexion() {
    std::cout << "Serveur listen on port 4148" << std::endl;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erreur lors de l'initialisation de Winsock." << std::endl;
        return;
    }

    SOCKET sock;
    SOCKADDR_IN sin;
    SOCKADDR_IN csin;
    int sizeof_csin = sizeof(csin);

    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_family = AF_INET;
    sin.sin_port = htons(4148); // Correction du port

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        WSACleanup();
        return;
    }

    if (bind(sock, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        std::cerr << "Erreur lors du binding du socket." << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) { // Correction de listen()
        std::cerr << "Erreur lors de la mise en écoute du socket." << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    while (true) {
        SOCKET clientSocket = accept(sock, (SOCKADDR*)&csin, &sizeof_csin);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Erreur lors de l'acceptation de la connexion." << std::endl;
            closesocket(sock);
            WSACleanup();
            return;
        }

        std::cout << "Client connecté." << std::endl;
        // Traitement de la connexion avec le client

        // Fermeture de la connexion du client
        closesocket(clientSocket);
    }

    // Le code ne passe jamais ici, mais il est bon de nettoyer même s'il est inaccessible
    closesocket(sock);
    WSACleanup();
}
