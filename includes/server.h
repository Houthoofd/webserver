#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <string>
#include <winsock2.h>

#ifndef SERVER_H
#define SERVER_H


class Server {
public:
    Server(int port = 8080);
    ~Server();

    void Run();
    void Stop();

private:
    int port;
    SOCKET listeningSocket;
    std::vector<SOCKET> clientSockets;

    bool Initialize();
    bool CreateListeningSocket();
    void Cleanup();
    std::string GetFileNameFromPath(const std::string& path);
    bool ends_with(const std::string& str, const std::string& suffix);
    void ServeFile(SOCKET clientSocket, const std::string& filePath);
    std::string ParseRequest(const std::string& request);
    static DWORD WINAPI HandleClientMessagesWrapper(LPVOID arg);
    void HandleClientMessages(SOCKET clientSocket);
    void SendMessage(SOCKET clientSocket, const std::string& message);

    bool IsPostRequest(const std::string& request);
    std::string ExtractPostData(const std::string& request);
    std::string GetRequestURL(const std::string& request);
};

#endif  // SERVER_H





