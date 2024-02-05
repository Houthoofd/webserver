#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <winsock2.h>

using namespace std;

std::string GetFileNameFromPath(const std::string& filePath) {
    size_t lastSeparatorPos = filePath.find_last_of("/\\");
    if (lastSeparatorPos != std::string::npos) {
        return filePath.substr(lastSeparatorPos + 1);
    }
    return filePath;
}

void EnregistrerDansFichiers(int length) {
    
}