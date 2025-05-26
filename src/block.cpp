#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <sys/types.h>
#include "block.h"

// std::string file_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/resource/no2block.json"; 


block::block(): previous_block_hash(""),
                merkel_root(""),
                time(0),
                nonce(0),
                version(0),
                bits(0),
                target(""){
    
};

std::string block::intToBits(int input){
    std::string result;
    char bytes[sizeof(int)];
    memcpy(bytes, reinterpret_cast<char*>(&input), sizeof(int));
    result = bytes;
    return result;
};


bool block::parseFromjson(rapidjson::Value& block_json){
    if(block_json.Empty()){
        std::cerr << "block_json is empty" << std::endl;
        return false;
    } 
    
    const rapidjson::Value& firstElement = block_json[0u];
    if(!firstElement.HasMember("ver") || !firstElement["ver"].IsInt()){
        std::cerr << "no version" << std::endl;
        return false;
    }

    if(!firstElement.HasMember("prev_block") || !firstElement["prev_block"].IsString()){
        std::cerr << "no previous block hash" << std::endl;
        return false;
    }

    if(!firstElement.HasMember("mrkl_root") || !firstElement["mrkl_root"].IsString()){
        std::cerr << "no mrkl_root" << std::endl;
        return false;
    }
    
    if(!firstElement.HasMember("time") || !firstElement["time"].IsInt()){
        std::cerr << "no time stamp" << std::endl;
        return false;
    }

    if(!firstElement.HasMember("bits") || !firstElement["bits"].IsInt()){
        std::cerr << "no bits for target" << std::endl;
        return false;
    }
    
    version = firstElement["ver"].GetInt();
    previous_block_hash = firstElement["prev_block"].GetString();
    merkel_root = firstElement["mrkl_root"].GetString();
    time = firstElement["time"].GetInt();
    bits = firstElement["bits"].GetInt();
    nonce = 2875051877;
    
    return true;
};

std::string block::parseStringToBytes(std::string input){
    if(input.size() != 64){
        std::cerr << "the string size is wrong" << std::endl;
        return "";
    }
    std::stringstream temp_stream;
    if(previous_block_hash.size() != 64){
        std::cerr << "block hash size wrong" << std::endl;
    }
    for (size_t i = 0; i < input.length(); i += 2) {
        u_int8_t hight = input[i];
        u_int8_t low = (i+1) < input.size() ? input[i+1] : 0;
        if(hight - '0' > 9){
            hight -= 'a';
            hight += 10;
        }
        else{
            hight -= '0';
        }
        if(low - '0' > 9){
            low -= 'a';
            low += 10;
        }
        else{
            low -= '0';
        }
        hight <<= 4;
        hight |= low;

        temp_stream << std::hex << hight;
    }
    std::string result = temp_stream.str();
    std::reverse(result.begin(), result.end());
    return result;
};

std::string block::parseIntToBytes(int input){
    std::stringstream temp_stream;
    unsigned char* p = reinterpret_cast<unsigned char*>(&input);
    // std::cout << "0x";
    temp_stream.clear();
    for(int i = 0; i < sizeof(int); i++){
        temp_stream << std::hex << static_cast<unsigned char>(p[i]);
    }
    
    return temp_stream.str();
}

bool block::parseAllStringToBytes(){
    previous_block_hash_bytes = parseStringToBytes(previous_block_hash);
    merkel_root_bytes = parseStringToBytes(merkel_root);
    if(previous_block_hash_bytes != "" && merkel_root != "") return true;
    return false;
}

bool block::parseAllIntToBytes(){
    version_bytes = parseIntToBytes(version);
    time_bytes = parseIntToBytes(time);
    bits_bytes = parseIntToBytes(bits);
    nonce_bytes = parseIntToBytes(nonce);
    if(version_bytes.empty() ||time_bytes.empty() || bits_bytes.empty() || nonce_bytes.empty()) return false;
    return true;
}

bool block::refreshNonceBytes(){
    if(nonce == 0xffffffff){
        return false;
    }
    nonce++;
    nonce_bytes = parseIntToBytes(nonce);
    return nonce_bytes.size() == 4? true : false;
}

bool block::createBlockJsonFromFile(std::string file_name){
    
    std::ifstream fin;
    fin.open(file_name,std::ios::in);
    if(!fin.is_open()){
        std::cerr<<"cannot open the file" << std::endl;
        return false;
    }

    std::stringstream V;

    V << fin.rdbuf();
    // 读json文件
    block_json = V.str();
    fin.close();

    return !block_json.empty() ? true : false;
}

bool block::parseAllFromJsonBlock(){
    if(block_json.empty()){
        std::cerr << "block json is empty" << std::endl;
        return false;
    }
    rapidjson::Document d;
    d.Parse(block_json.c_str());
    if (d.HasParseError()) {
        std::cerr << "rapidJSON parse error" << std::endl;
        return false;
    }
    // test 输出看正确与否
    rapidjson::Value& array = d["blocks"];
    if(array.Empty()){
        std::cerr << "block extract error" << std::endl;
        return false;    
    }

    if(!parseFromjson(array)){
        std::cerr << "block extract error" << std::endl;
        return false;    
    }
    // std::cout << "the preious_hash == " << test_block.merkel_root<< std::endl;
    if(!parseAllIntToBytes()){
        std::cerr << "parse INT error" << std::endl;
        return false;
    }
    if(!parseAllStringToBytes()){
        std::cerr << "parse String error" << std::endl;
        return false;
    }

    return true;
}


bool block::createTargetFromBits(){
    int exponent = bits & 0xff000000;
    exponent >>= 24;
    exponent -= 3;
    target.resize(32);
    u_int8_t* byte_pointer = reinterpret_cast<u_int8_t*>(&bits);
    for(int i = 0; i < 3; i++){
        target[31 - exponent -i] = byte_pointer[i];
    }
    return true;
}

// int main(){
    
//     block test;
//     if(!test.createBlockJsonFromFile(file_name)){
//         return -1;
//     }
//     if(!test.parseAllFromJsonBlock()){
//         return -1;
//     }
//     std::cout <<"the nonce is " << std::hex << test.nonce << std::endl;
//     for (unsigned char c : test.nonce_bytes) {
//     // 以十六进制形式输出每个字符的底层字节
//         std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//     }
//     std::cout << std::endl;

//     std::cout <<"the time is " << std::hex << test.time << std::endl;
//     for (unsigned char c : test.time_bytes) {
//     // 以十六进制形式输出每个字符的底层字节
//         std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//     }
//     std::cout << std::endl;
    
//     std::cout <<"the pre hash is " << std::hex << test.previous_block_hash << std::endl;
//     for (unsigned char c : test.previous_block_hash_bytes) {
//     // 以十六进制形式输出每个字符的底层字节
//         std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//     }
//     std::cout << std::endl;

//     std::cout <<"the merkel root is " << std::hex << test.merkel_root << std::endl;
//     for (unsigned char c : test.merkel_root_bytes) {
//     // 以十六进制形式输出每个字符的底层字节
//         std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//     }
//     std::cout << std::endl;

//     std::cout <<"the version is " << std::hex << test.version << std::endl;
//     for (unsigned char c : test.version_bytes) {
//     // 以十六进制形式输出每个字符的底层字节
//         std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//     }
//     std::cout << std::endl;
    
//     return 0;
// }


// std::cout << "0x" << std::endl;
//     for (unsigned char c : previous_block_hash_bytes) {
//     // 以十六进制形式输出每个字符的底层字节
//         std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//     }