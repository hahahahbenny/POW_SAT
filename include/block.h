#ifndef BLOCK
#define BLOCK

#include "rapidjson/document.h"
#include <string>
#include <cstring>

class block {
public:
    block();
    ~block() = default;
    std::string block_json;
    unsigned int version;
    std::string previous_block_hash;
    std::string merkel_root;
    unsigned int time;
    unsigned int bits;
    unsigned int nonce;

    std::string previous_block_hash_bytes;
    std::string merkel_root_bytes;
    std::string version_bytes;
    std::string time_bytes;
    std::string bits_bytes;
    std::string nonce_bytes;
    std::string target;
    
    bool createBlockJsonFromFile(std::string file_name);
    bool parseAllFromJsonBlock();
    std::string intToBits(int input);
    bool refreshNonceBytes();
    bool transBitsToTarget();
    bool createTargetFromBits();
    

private:
    bool parseAllStringToBytes();
    bool parseAllIntToBytes();
    bool parseFromjson(rapidjson::Value& block_json); 
    std::string parseStringToBytes(std::string input);
    std::string parseIntToBytes(int input);
};


#endif // !BLOCK