#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>
#include <iomanip>
#include <ostream>
#include <fstream>
#include "block.h"


int main(){

    
    std::string file_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/resource/no2block.json"; 
    std::string cnf_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/bitcoin.cnf";

    // open cnf file
    std::ofstream cnf_file;
    cnf_file.open(cnf_name);
    if(! cnf_file.is_open()){
        std::cerr << "open file failed" << std::endl;
    }

    // todo, 改进此处的输入和main函数， 保留tsetin编码和输出
    block block_mining;
    if(!block_mining.createBlockJsonFromFile(file_name)){
        return -1;
    }
    // std::cout << "after create from file" << std::endl;
    if(!block_mining.parseAllFromJsonBlock()){
        return -1;
    }

    if(!block_mining.createTargetFromBits()){
        std::cerr << "create from bits failed" << std::endl;
    }

    const u_int8_t* target = reinterpret_cast<const u_int8_t*>(block_mining.target.data());

    for (int i = 0; i < 32; i++) {
    // 以十六进制形式输出每个字符的底层字节
        std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(target[i]) << " ";
    }
    std::cout << std::endl;

    u_int8_t restrict[8][32];
    memset(restrict, -1, 256);

    
    for(int i = 0; i < 32; i++){
        uint8_t temp = target[i];
       
        if(temp != 0){
            for(int j = 0; j < 8; j++){ 
                bool left = temp & 0x80;
                restrict[7-(i/4)][(3-i%4)*8 + j] = 0;
                if(left) break;
                temp <<= 1;
            }
            break;
        }

        for (unsigned int j = 0; j < 8; ++j)
        {
            restrict[7-(i/4)][(3 - i%4)*8 + j] = 0;
        }
        std::cout << "now " << i << "time" << "\n";
    }


    for(int i = 0; i < 256; i++){
        if(i % 8 == 0) std::cout << '\n';
        std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*(*(restrict)+i)) << " ";
        
    }
    std::cout << "\n";
    

}

