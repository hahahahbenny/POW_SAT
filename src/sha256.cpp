#include "sha256.h"
#include <iostream>
#include <iomanip>
#include <ostream>

std::string sha256(const std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    // std::stringstream ss;
    std::string result(reinterpret_cast<const char*>(hash), SHA256_DIGEST_LENGTH);
    

    return result;
}

