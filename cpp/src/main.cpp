#include <iostream>

#include "ram/account.h"
#include "ram/ini_file.h"
#include "ram/utilities.h"

int main() {
    std::cout << "Roblox Account Manager (C++ Edition)" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "Core library loaded successfully." << std::endl;
    std::cout << "MD5 of 'test': " << ram::md5("test") << std::endl;
    return 0;
}
