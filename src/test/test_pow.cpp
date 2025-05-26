#include "sat/pow.h"
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

int num = 0;

void newVars(std::string label, int x[], unsigned int n){
    for (unsigned int i = 0; i < n; ++i)
        x[i] = ++num;
}

int main(int argc, const char** argv) {
    std::string file_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/resource/latestblock.json"; 
    std::string cnf_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/OptimizeVars_a_to_h/";
    // PowCnf c;
    // if(!c.initBlockFromFile(file_name)){
    //     std::cout << "init block from file failed" << std::endl;
    // }
    std::vector<uint32_t> target = {0xc7437a17, 0x43fbcfe3, 0x9f295d24, 0x54eb7063, 0xcc96d219, 0x944d0100, 0x00000000, 0x00000000};
    // std::vector<uint32_t> target = {0xc1e789b8, 0xdcdc20a0, 0xb83e54b4, 0x58caeb90, 0x0cb68600, 0xc012b938, 0x29c3da5a, 0x13d9cf02};

    PowCnf t(file_name);
    t.printMiningBlock();
    // 此处之前没问题
    t.generateCnfFrameFromBlock();
    // t.setHout(target);
    t.constrainHout();
    for(int i = 16; i >= 4; i--){
        t.ConstrainNonce( 0x65cf5dab, i);
        std::string left_bit = (std::to_string(i) + "bits_left.cnf");
        if(!t.writeFile(cnf_name + left_bit)){
            std::cerr << "write cnf file failed" << std::endl;
        }
    }
    
    t.printVariablesAndClauseNum("done");
    // c.printMiningBlock();
    
    // int A_round1[1 + 64][32];
    // int B_round1[1 + 64][32];
    // int C_round1[1 + 64][32];
    // int D_round1[1 + 64][32];
    // int E_round1[1 + 64][32];
    // int F_round1[1 + 64][32];
    // int G_round1[1 + 64][32];
    // int H_round1[1 + 64][32];

    // // round2
    // int A_round2[1 + 64][32];
    // int B_round2[1 + 64][32];
    // int C_round2[1 + 64][32];
    // int D_round2[1 + 64][32];
    // int E_round2[1 + 64][32];
    // int F_round2[1 + 64][32];
    // int G_round2[1 + 64][32];
    // int H_round2[1 + 64][32];


    // for(int i = 0; i < 16; i++){
    //     int start_pos = i*4;
    //     newVars("a_round1_" + std::to_string(start_pos), A_round1[start_pos], 32);
    //     newVars("b_round1_" + std::to_string(start_pos), B_round1[start_pos], 32);
    //     newVars("c_round1_" + std::to_string(start_pos), C_round1[start_pos], 32);
    //     newVars("d_round1_" + std::to_string(start_pos), D_round1[start_pos], 32);
    //     newVars("e_round1_" + std::to_string(start_pos), E_round1[start_pos], 32);
    //     newVars("f_round1_" + std::to_string(start_pos), F_round1[start_pos], 32);
    //     newVars("g_round1_" + std::to_string(start_pos), G_round1[start_pos], 32);
    //     newVars("h_round1_" + std::to_string(start_pos), H_round1[start_pos], 32);
    //     newVars("a_round2_" + std::to_string(start_pos), A_round2[start_pos], 32);
    //     newVars("b_round2_" + std::to_string(start_pos), B_round2[start_pos], 32);
    //     newVars("c_round2_" + std::to_string(start_pos), C_round2[start_pos], 32);
    //     newVars("d_round2_" + std::to_string(start_pos), D_round2[start_pos], 32);
    //     newVars("e_round2_" + std::to_string(start_pos), E_round2[start_pos], 32);
    //     newVars("f_round2_" + std::to_string(start_pos), F_round2[start_pos], 32);
    //     newVars("g_round2_" + std::to_string(start_pos), G_round2[start_pos], 32);
    //     newVars("h_round2_" + std::to_string(start_pos), H_round2[start_pos], 32);

    //     for(int j = 1; j < 4; j++){
    //         switch (j) {
    //         case 1: memcpy(B_round1[start_pos+j], A_round1[start_pos+j-1], 32);  
    //                 memcpy(F_round1[start_pos+j], E_round1[start_pos+j-1], 32);   
    //                 memcpy(B_round2[start_pos+j], A_round2[start_pos+j-1], 32);  
    //                 memcpy(F_round2[start_pos+j], E_round2[start_pos+j-1], 32);   

    //         case 2: memcpy(C_round1[start_pos+j], B_round1[start_pos+j-1], 32);         
    //                 memcpy(G_round1[start_pos+j], F_round1[start_pos+j-1], 32);    
    //                 memcpy(C_round2[start_pos+j], B_round2[start_pos+j-1], 32);  
    //                 memcpy(G_round2[start_pos+j], F_round2[start_pos+j-1], 32);   
            
    //         case 3: memcpy(H_round1[start_pos+j], G_round1[start_pos+j-1], 32);      
    //                 memcpy(D_round1[start_pos+j], C_round1[start_pos+j-1], 32);
    //                 memcpy(H_round2[start_pos+j], G_round2[start_pos+j-1], 32);      
    //                 memcpy(D_round2[start_pos+j], C_round2[start_pos+j-1], 32);
    //         }
    //         switch (j) {
    //         case 3: newVars("c_round1_" + std::to_string(start_pos+j), C_round1[start_pos+j], 32);
    //                 newVars("g_round1_" + std::to_string(start_pos+j), G_round1[start_pos+j], 32);
    //                 newVars("c_round2_" + std::to_string(start_pos+j), C_round2[start_pos+j], 32);
    //                 newVars("g_round2_" + std::to_string(start_pos+j), G_round2[start_pos+j], 32);

    //         case 2: newVars("b_round1_" + std::to_string(start_pos+j), B_round1[start_pos+j], 32);
    //                 newVars("f_round1_" + std::to_string(start_pos+j), F_round1[start_pos+j], 32);
    //                 newVars("b_round2_" + std::to_string(start_pos+j), B_round2[start_pos+j], 32);
    //                 newVars("f_round2_" + std::to_string(start_pos+j), F_round2[start_pos+j], 32);

    //         case 1: newVars("a_round1_" + std::to_string(start_pos+j), A_round1[start_pos+j], 32);
    //                 newVars("e_round1_" + std::to_string(start_pos+j), E_round1[start_pos+j], 32);
    //                 newVars("a_round2_" + std::to_string(start_pos+j), A_round2[start_pos+j], 32);
    //                 newVars("e_round2_" + std::to_string(start_pos+j), E_round2[start_pos+j], 32);
    //         }
    //     }
    // }
    
    // for(int i = 0; i < 8; i++){
    //     std::cout << A_round1[i][0] << " ";
    //     std::cout << B_round1[i][0] << " ";
    //     std::cout << C_round1[i][0] << " ";
    //     std::cout << D_round1[i][0] << " ";
    //     std::cout << E_round1[i][0] << " ";
    //     std::cout << F_round1[i][0] << " ";
    //     std::cout << G_round1[i][0] << " ";
    //     std::cout << H_round1[i][0] << " ";
    //     std::cout << std::endl;
    // }

    // for(int i = 16; i < 64; i++){
    //     int pos_1 = i-2;
    //     int pos_2 = i-7;
    //     int pos_3 = i-15;
    //     int pos_4 = i-16;
    //     std::cout << i << ":" << ((pos_1 >=5 && pos_1 <= 14) ? "*" : "" )<<"sigma1(" << pos_1 << ") " 
    //               <<((pos_1 >=5 && pos_2 <= 14) ? "*" : "" )<< pos_2 << 
    //               ((pos_3 >=5 && pos_3 <= 14) ? "*" : "" ) << " sigma0(" << pos_3 << ") " << 
    //               ((pos_4 >=5 && pos_4 <= 14) ? "*" : "" ) << pos_4 << std::endl;     
    // }


    return 0;
}