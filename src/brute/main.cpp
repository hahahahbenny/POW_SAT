#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/types.h>
#include "sha256.h"
#include "block.h"
#include <iomanip>
#include <thread>
#include <atomic>
#include <chrono> 

bool cmpTargetAndSha256(std::string Target, std::string input);

std::string file_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/resource/no2block.json"; 



std::atomic<bool> should_exit(false); // 标志位，用于通知主循环退出

void timer_thread(std::chrono::seconds max_duration) {
    auto start_time = std::chrono::high_resolution_clock::now();
    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);

        if (elapsed_time >= max_duration) {
            should_exit = true; // 设置退出标志
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1)); // 每秒检查一次
    }
}



int main(){
    block block_mining;
    if(!block_mining.createBlockJsonFromFile(file_name)){
        return -1;
    }
    // std::cout << "after create from file" << std::endl;
    if(!block_mining.parseAllFromJsonBlock()){
        return -1;
    }
    // std::cout << "after parse" << std::endl;
    std::string sha_result(32, 0xff);
    std::string input_except_nonce = block_mining.version_bytes;
    input_except_nonce += block_mining.previous_block_hash_bytes;
    input_except_nonce += block_mining.merkel_root_bytes;
    input_except_nonce += block_mining.time_bytes;
    input_except_nonce += block_mining.bits_bytes;
   
    if(!block_mining.createTargetFromBits()){
        std::cerr << "create from bits failed" << std::endl;
    }

    const std::chrono::seconds max_duration(10); // 10 分钟
     std::thread timer(timer_thread, max_duration);
    // // sha256("abc");
    // std::cout << std::endl;
    sha_result = opensslSha256(input_except_nonce + block_mining.nonce_bytes);
    sha_result = opensslSha256(sha_result);

    std::reverse(sha_result.begin(), sha_result.end());    
    while(!should_exit){
        if(cmpTargetAndSha256(block_mining.target, sha_result)){
            std::cout << "get the right nonce" << std::endl; 
            break;
        }
        if(!block_mining.refreshNonceBytes()){
            std::cerr << "nonce get max or trans to byte error" << std::endl;
        }
        printf("the nonce is %d\n", block_mining.nonce);
        sha_result = opensslSha256(input_except_nonce + block_mining.nonce_bytes);
        sha_result = opensslSha256(sha_result);
        std::reverse(sha_result.begin(), sha_result.end());   
    };
    std::cout << "Time limit reached. Exiting..." << std::endl;
    
    std::cout << "caculate ok" << std::endl;
    std::cout << "the nonce is " << block_mining.nonce << std::endl;
    // std::cout << "the sha256 is " << std::endl;
    // std::cout << "0x";
    // for (unsigned char c : sha_result) {
    // // 以十六进制形式输出每个字符的底层字节
    //     std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    // }
    // std::cout << std::endl;

    return 0;
};


bool cmpTargetAndSha256(std::string Target, std::string input){

    return input < Target;
}

// std::cout << "the target is " << std::endl;
//     std::cout << "0x" << std::endl;
//     for (unsigned char c : target) {
//     // 以十六进制形式输出每个字符的底层字节
//         std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
//     }
//     std::cout << std::endl;