#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

#include "file.h"
#include "functions.h"
#include "server.h"

using namespace std;

Server::Server(int port) : port(port), listeningSocket(INVALID_SOCKET) {}

Server::~Server() {
    Stop();
}

void Server::Run() {
    if (!Initialize()) {
        std::cerr << "Erreur lors de l'initialisation de Winsock." << std::endl;
        return;
    }

    if (!CreateListeningSocket()) {
        std::cerr << "Erreur lors de la création du socket d'écoute." << std::endl;
        Cleanup();
        return;
    }

    std::cout << "Le serveur écoute sur le port " << port << "." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Erreur lors de l'acceptation d'une connexion client." << std::endl;
            continue;
        }

        std::cout << "Nouvelle connexion client acceptée. Socket : " << clientSocket << std::endl;

        // Recevoir la requête du client
        char buffer[4096];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            std::string request(buffer, bytesRead);

            std::string jsonData;
            bool isInJsonData = false;

            for (char c : request) {
                if (!isInJsonData && c == '{') {
                    isInJsonData = true;
                } else if (isInJsonData && c == '}') {
                    isInJsonData = false;
                    // Vous avez extrait les données JSON, vous pouvez les traiter ici
                    std::cout << "Données JSON : " << jsonData << std::endl;

        
                    std::size_t valueStart = jsonData.find("\"value\":\"") + 9;  // Position de début de la valeur
                    std::size_t valueEnd = jsonData.find("\"", valueStart);  // Position de fin de la valeur
                    std::string extractedValue = jsonData.substr(valueStart, valueEnd - valueStart);
                    std::cout << "Valeur extraite : " << extractedValue << std::endl;

                    // Convertir la valeur en int
                    int extractedIntValue = std::stoi(extractedValue);
                    RunProgram(extractedIntValue);

                    // Réinitialisez la chaîne jsonData pour la prochaine utilisation
                    jsonData.clear();
                } else if (isInJsonData) {
                    jsonData += c;
                } else {
                    // Autres traitements de caractères non inclus dans les données JSON
                }
            }

            // Analyser la requête pour obtenir le chemin du fichier demandé
            std::string filePath = ParseRequest(request);
            if (!filePath.empty()) {
                if (IsPostRequest(request) && GetRequestURL(request) == "/interface/resultats.html") {
                    // Envoyer une réponse au client
                    std::string response = "HTTP/1.1 200 OK\r\n";
                    response += "Content-Type: text/plain\r\n";
                    response += "Content-Length: 12\r\n";
                    response += "\r\n";
                    response += "Hello, World!";
                    send(clientSocket, response.c_str(), response.size(), 0);
                } else {
                    // Traiter la requête GET normalement
                    ServeFile(clientSocket, filePath);
                }
            }
        }

        // Fermer la connexion client
        closesocket(clientSocket);
    }

    Cleanup();
}







bool Server::IsPostRequest(const std::string& request) {
    // Check if the request method is "POST"
    std::istringstream iss(request);
    std::string method;
    iss >> method;
    return (method == "POST");
}

std::string Server::ExtractPostData(const std::string& request) {
    // Find the position of the double newline indicating the start of the POST data
    std::size_t dataStart = request.find("\r\n\r\n");
    if (dataStart != std::string::npos) {
        // Extract the POST data
        std::string postData = request.substr(dataStart + 4);
        return postData;
    }
    return "";
}


std::string Server::ParseRequest(const std::string& request) {
    std::string filePath;

    // Extraire l'URL de la requête
    std::istringstream iss(request);
    std::string method, url, version;
    iss >> method >> url >> version;

    // Vérifier si la méthode de requête est GET et extraire le chemin du fichier demandé
    if (method == "GET") {
        // Supprimer les paramètres de l'URL (le cas échéant)
        std::size_t paramPos = url.find('?');
        if (paramPos != std::string::npos) {
            url = url.substr(0, paramPos);
        }

        // Supprimer le premier '/' du chemin de fichier
        if (url.length() > 1 && url[0] == '/') {
            url = url.substr(1);
        }

        // Utiliser le chemin du fichier demandé
        filePath = url;
    }

    return filePath;
}

void Server::Stop() {
    for (SOCKET clientSocket : clientSockets) {
        closesocket(clientSocket);
    }

    clientSockets.clear();

    if (listeningSocket != INVALID_SOCKET) {
        closesocket(listeningSocket);
        listeningSocket = INVALID_SOCKET;
    }

    Cleanup();
}

bool Server::Initialize() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void Server::Cleanup() {
    WSACleanup();
}

std::string Server::GetFileNameFromPath(const std::string& path) {
    // Recherche du dernier séparateur de chemin
    size_t lastSeparator = path.find_last_of("/\\");
    if (lastSeparator != std::string::npos) {
        // Retourne la sous-chaîne après le dernier séparateur
        return path.substr(lastSeparator + 1);
    }

    // Si aucun séparateur n'est trouvé, retourne le chemin complet
    return path;
}

bool Server::ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void Server::ServeFile(SOCKET clientSocket, const std::string& filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    std::string fileContent;

    if (file) {
        std::ostringstream oss;
        oss << file.rdbuf();
        fileContent = oss.str();
    }

    // Déterminer le type de contenu en fonction de l'extension du fichier
    std::string contentType;
    if (ends_with(filePath, ".html")) {
        contentType = "text/html";
    } else if (ends_with(filePath, ".css")) {
        contentType = "text/css";
    } else if (ends_with(filePath, ".js")) {
        contentType = "application/javascript";
    }

    // Envoyer le contenu du fichier au client
    if (!fileContent.empty()) {
        // Envoyer les en-têtes de la réponse HTTP
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + contentType + "\r\n";
        response += "Content-Length: " + std::to_string(fileContent.size()) + "\r\n";
        response += "\r\n";

        // Envoyer les en-têtes suivis du contenu du fichier
        send(clientSocket, response.c_str(), response.size(), 0);
        send(clientSocket, fileContent.c_str(), fileContent.size(), 0);
    } else {
        // Fichier non trouvé, renvoyer une réponse 404 Not Found
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
    }
}


bool Server::CreateListeningSocket() {
    listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket == INVALID_SOCKET) {
        std::cerr << "Erreur lors de la création du socket d'écoute." << std::endl;
        return false;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(listeningSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Erreur lors de l'association du socket d'écoute avec le port." << std::endl;
        closesocket(listeningSocket);
        return false;
    }

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Erreur lors de la mise en écoute du socket." << std::endl;
        closesocket(listeningSocket);
        return false;
    }

    return true;
}

DWORD WINAPI Server::HandleClientMessagesWrapper(LPVOID arg) {
    Server* server = static_cast<Server*>(arg);
    SOCKET clientSocket = server->clientSockets.back();
    server->clientSockets.pop_back();
    server->HandleClientMessages(clientSocket);  // Appel de la fonction membre avec le paramètre clientSocket
    return 0;
}

void Server::HandleClientMessages(SOCKET clientSocket) {
    clientSockets.pop_back();  // Supprimer le socket client de la liste

    char buffer[4096];
    std::string message;

    while (true) {
        try {
            memset(buffer, 0, sizeof(buffer));

            int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead <= 0) {
                // Erreur ou connexion fermée par le client
                std::cout << "Client déconnecté. Socket : " << clientSocket << std::endl;
                closesocket(clientSocket);
                break;
            }

            message = buffer;

            // Traitement du message reçu
            // ...

            // Exemple : renvoyer le message au client
            SendMessage(clientSocket, message);

            // Exemple : gestion des requêtes HTTP
            // Supposons que la requête GET "/resultats.html" demande le fichier 'resultats.html'
            if (message.find("GET") != std::string::npos) {
                std::size_t start = message.find_first_of(' ') + 1;
                std::size_t end = message.find_first_of(' ', start);
                std::string requestedFile = message.substr(start, end - start);

                cout << requestedFile << endl;
                // Si le fichier demandé est "/resultats.html"
                if (requestedFile == "/resultats.html") {
                    std::string filename = GetFileNameFromPath(requestedFile);
                    std::ifstream file("interface/" + filename, std::ios::in | std::ios::binary);
                    if (file) {
                        std::string fileContent;

                        // Lire le contenu du fichier ligne par ligne
                        std::string line;
                        while (std::getline(file, line)) {
                            fileContent += line + "\n";
                        }

                        // Envoyer les en-têtes de la réponse HTTP
                        std::string response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: text/html\r\n";
                        response += "Content-Length: " + std::to_string(fileContent.size()) + "\r\n";
                        response += "\r\n";

                        // Envoyer les en-têtes suivis du contenu du fichier
                        send(clientSocket, response.c_str(), response.size(), 0);
                        send(clientSocket, fileContent.c_str(), fileContent.size(), 0);
                    } else {
                        // Fichier non trouvé, renvoyer une réponse 404 Not Found
                        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
                        send(clientSocket, response.c_str(), response.size(), 0);
                    }
                }
            }
        } catch (const std::exception& e) {
            // Gérer l'exception et afficher un message d'erreur
            std::cerr << "Erreur lors du traitement de la connexion client : " << e.what() << std::endl;
            closesocket(clientSocket);
            break;
        }
    }
}




void Server::SendMessage(SOCKET clientSocket, const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);
}

std::string Server::GetRequestURL(const std::string& request) {
    // Expression régulière pour extraire l'URL de la requête HTTP
    std::regex urlRegex(R"(GET\s+([^\s]+))");
    std::smatch match;

    if (std::regex_search(request, match, urlRegex)) {
        if (match.size() > 1) {
            return match[1].str(); // Renvoie la première capture de groupe correspondante (l'URL)
        }
    }

    return ""; // Retourne une chaîne vide par défaut si l'extraction échoue ou si aucune correspondance n'est trouvée
}



