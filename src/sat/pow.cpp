#include "sat/pow.h"
#include "sat/format.hh"
#include "block.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#define ROUND2

template <typename T>
void outPutHex(std::string title, T* arr, size_t size){
    std::cout <<title << std::hex  << std::endl;
    for (int i = 0; i < size; i++) {
    // 以十六进制形式输出每个字符的底层字节
        std::cout << std::hex << std::setw(sizeof(T) * 2) << std::setfill(' ') 
                      << static_cast<uint32_t>(static_cast<typename std::make_unsigned<T>::type>(arr[i])) << " ";
    }
    std::cout << "\n";
}

void PowCnf::constantWithExist(int r[], u_int32_t value, std::string help_text, int size){
    for(int i = 0; i < size; i++){
        r[i] = ((value >> i) & 0x1) ? constant_bool[1] : constant_bool[0];
    }
}

bool PowCnf::assignVariablesIndices(){  
    // todo
    // 2. 只分配需要新变量的部分
    // 3. 常量全部用最开始的0和1来代替
    // 所以最终需要分配的就只有几个变量
    // w_round1[3]
    // w16-w63
    // a1h1-a64h64
    // sigma之类的中间变量，这个可能还可以优化
    newVars("constant vars", constant_bool, 2);
    constant1(constant_bool[0], 0, "constant 0 variable");
    constant1(constant_bool[1], 1, "constant 1 variable");

    
    newVars("nonce_w_round1[3]", w_round1[3], 32);
    for(int i = 0; i < 64; i++){
        if(i >= 16){
            newVars("init W_round1["+ std::to_string(i)+ "]", w_round1[i], 32);
        }

#ifdef ROUND2
        if(i < 8 || i >= 16){
            newVars("init W_round2["+ std::to_string(i)+ "]", w_round2[i], 32);
        }
#endif 
    }

    // 此处因为要先用存在的变量先初始化最初的第一轮变量才能让后面的传播成为可能
    std::vector<uint32_t> first_round_output;
    first_round_output.resize(8);
    uint8_t input[64];
    for(int i = 0; i < 4; i++){
        input[i] = mining_block.version_bytes[i];
    }
    for(int i = 4; i < 36; i++){
        input[i] = mining_block.previous_block_hash_bytes[i-4];
    }
    for(int i = 36; i < 64; i++){
        input[i] = mining_block.merkel_root_bytes[i-36];
    }
    sha256_forward(input, 0, first_round_output.data());
    constantAToH(first_round_output);

    //初始化轮函数的变量 a-h 的变量分配 初始化第一轮变量
//     comment("init a-h varialble");
//     newVars("a0_round1", A_round1[0], 32);
//     newVars("b0_round1", B_round1[0], 32);
//     newVars("c0_round1", C_round1[0], 32);
//     newVars("d0_round1", D_round1[0], 32);
//     newVars("e0_round1", E_round1[0], 32);
//     newVars("f0_round1", F_round1[0], 32);
//     newVars("g0_round1", G_round1[0], 32);
//     newVars("h0_round1", H_round1[0], 32);
// #ifdef ROUND2
//     newVars("a0_round2", A_round2[0], 32);
//     newVars("b0_round2", B_round2[0], 32);
//     newVars("c0_round2", C_round2[0], 32);
//     newVars("d0_round2", D_round2[0], 32);
//     newVars("e0_round2", E_round2[0], 32);
//     newVars("f0_round2", F_round2[0], 32);
//     newVars("g0_round2", G_round2[0], 32);
//     newVars("h0_round2", H_round2[0], 32);

    // 
// #endif 
    for(int i = 1; i <= 64; i++){
        // 每轮其实都是直接新变量
        newVars("a"+std::to_string(i)+"round1", A_round1[i], 32);
        newVars("e"+std::to_string(i)+"round1", E_round1[i], 32);

        memcpy(B_round1[i], A_round1[i-1], 32 * sizeof(int));  
        memcpy(C_round1[i], B_round1[i-1], 32 * sizeof(int)); 
        memcpy(D_round1[i], C_round1[i-1], 32 * sizeof(int));
        memcpy(F_round1[i], E_round1[i-1], 32 * sizeof(int));
        memcpy(G_round1[i], F_round1[i-1], 32 * sizeof(int));
        memcpy(H_round1[i], G_round1[i-1], 32 * sizeof(int));  

        // 分配轮函数中计算T1和T2的中间变量
        newVars("init tmp1_round1",  tmp1_round1[i], 32);
        newVars("init tmp1a_round1", tmp1a_round1[i], 32);
        newVars("init tmp1b_round1", tmp1b_round1[i], 32);
        newVars("init tmp2_round1",  tmp2_round1[i], 32);
        newVars("init tmp2a_round1", tmp2a_round1[i], 32);
        newVars("init tmp2b_round1", tmp2b_round1[i], 32);

#ifdef ROUND2
        newVars("a"+std::to_string(i)+"round2", A_round2[i], 32);
        newVars("e"+std::to_string(i)+"round2", E_round2[i], 32);

        memcpy(B_round2[i], A_round2[i-1], 32 * sizeof(int));  
        memcpy(C_round2[i], B_round2[i-1], 32 * sizeof(int)); 
        memcpy(D_round2[i], C_round2[i-1], 32 * sizeof(int));
        memcpy(F_round2[i], E_round2[i-1], 32 * sizeof(int));
        memcpy(G_round2[i], F_round2[i-1], 32 * sizeof(int));
        memcpy(H_round2[i], G_round2[i-1], 32 * sizeof(int));   

        newVars("init tmp1_round2",  tmp1_round2[i], 32);
        newVars("init tmp1a_round2", tmp1a_round2[i], 32);
        newVars("init tmp1b_round2", tmp1b_round2[i], 32);
        newVars("init tmp2_round2",  tmp2_round2[i], 32);
        newVars("init tmp2a_round2", tmp2a_round2[i], 32);
        newVars("init tmp2b_round2", tmp2b_round2[i], 32);  
#endif 
    }

    // 分配最终的h_out
    for(int i = 0; i < 8; i++){
        newVars("init h_out[" + std::to_string(i) + "]", h_out[i], 32);
    }
    
    return true;
}

void PowCnf::constantK(){
    //todo: 使用constantwithexist来代替这个
    //

    constantWithExist(k[0], 0x428a2f98, "k0", 32);
    constantWithExist(k[1], 0x71374491, "k1", 32);
    constantWithExist(k[2], 0xb5c0fbcf, "k2", 32);
    constantWithExist(k[3], 0xe9b5dba5, "k3", 32);
    constantWithExist(k[4], 0x3956c25b, "k4", 32);

    constantWithExist(k[5], 0x59f111f1, "k5", 32);
    constantWithExist(k[6], 0x923f82a4, "k6", 32);
    constantWithExist(k[7], 0xab1c5ed5, "k7", 32);
    constantWithExist(k[8], 0xd807aa98, "k8", 32);
    constantWithExist(k[9], 0x12835b01, "k9", 32);

    constantWithExist(k[10], 0x243185be, "k10", 32);
    constantWithExist(k[11], 0x550c7dc3, "k11", 32);
    constantWithExist(k[12], 0x72be5d74, "k12", 32);
    constantWithExist(k[13], 0x80deb1fe, "k13", 32);
    constantWithExist(k[14], 0x9bdc06a7, "k14", 32);

    constantWithExist(k[15], 0xc19bf174, "k15", 32);
    constantWithExist(k[16], 0xe49b69c1, "k16", 32);
    constantWithExist(k[17], 0xefbe4786, "k17", 32);
    constantWithExist(k[18], 0x0fc19dc6, "k18", 32);
    constantWithExist(k[19], 0x240ca1cc, "k19", 32);

    constantWithExist(k[20], 0x2de92c6f, "k20", 32);
    constantWithExist(k[21], 0x4a7484aa, "k21", 32);
    constantWithExist(k[22], 0x5cb0a9dc, "k22", 32);
    constantWithExist(k[23], 0x76f988da, "k23", 32);
    constantWithExist(k[24], 0x983e5152, "k24", 32);

    constantWithExist(k[25], 0xa831c66d, "k25", 32);
    constantWithExist(k[26], 0xb00327c8, "k26", 32);
    constantWithExist(k[27], 0xbf597fc7, "k27", 32);
    constantWithExist(k[28], 0xc6e00bf3, "k28", 32);
    constantWithExist(k[29], 0xd5a79147, "k29", 32);

    constantWithExist(k[30], 0x06ca6351, "k30", 32);
    constantWithExist(k[31], 0x14292967, "k31", 32);
    constantWithExist(k[32], 0x27b70a85, "k32", 32);
    constantWithExist(k[33], 0x2e1b2138, "k33", 32);
    constantWithExist(k[34], 0x4d2c6dfc, "k34", 32);

    constantWithExist(k[35], 0x53380d13, "k35", 32);
    constantWithExist(k[36], 0x650a7354, "k36", 32);
    constantWithExist(k[37], 0x766a0abb, "k37", 32);
    constantWithExist(k[38], 0x81c2c92e, "k38", 32);
    constantWithExist(k[39], 0x92722c85, "k39", 32);

    constantWithExist(k[40], 0xa2bfe8a1, "k40", 32);
    constantWithExist(k[41], 0xa81a664b, "k41", 32);
    constantWithExist(k[42], 0xc24b8b70, "k42", 32);
    constantWithExist(k[43], 0xc76c51a3, "k43", 32);
    constantWithExist(k[44], 0xd192e819, "k44", 32);

    constantWithExist(k[45], 0xd6990624, "k45", 32);
    constantWithExist(k[46], 0xf40e3585, "k46", 32);
    constantWithExist(k[47], 0x106aa070, "k47", 32);
    constantWithExist(k[48], 0x19a4c116, "k48", 32);
    constantWithExist(k[49], 0x1e376c08, "k49", 32);

    constantWithExist(k[50], 0x2748774c, "k50", 32);
    constantWithExist(k[51], 0x34b0bcb5, "k51", 32);
    constantWithExist(k[52], 0x391c0cb3, "k52", 32);
    constantWithExist(k[53], 0x4ed8aa4a, "k53", 32);
    constantWithExist(k[54], 0x5b9cca4f, "k54", 32);

    constantWithExist(k[55], 0x682e6ff3, "k55", 32);
    constantWithExist(k[56], 0x748f82ee, "k56", 32);
    constantWithExist(k[57], 0x78a5636f, "k57", 32);
    constantWithExist(k[58], 0x84c87814, "k58", 32);
    constantWithExist(k[59], 0x8cc70208, "k59", 32);

    constantWithExist(k[60], 0x90befffa, "k60", 32);
    constantWithExist(k[61], 0xa4506ceb, "k61", 32);
    constantWithExist(k[62], 0xbef9a3f7, "k62", 32);
    constantWithExist(k[63], 0xc67178f2, "k63", 32);
}

void PowCnf::constantW_round1(){
    // todo将constant和constant32只用一个变量来表示
    
    std::string temp = mining_block.merkel_root_bytes;
    temp += mining_block.time_bytes;
    temp += mining_block.bits_bytes;

    for(int i = 0; i < 12; i++){
        constantWithExist(&(w_round1[i/4][24-(8*(i%4))]), temp[i+28], "" , 8);
    }

    constantWithExist(w_round1[4], 0x80000000,  "w_round1_4", 32);
    constantWithExist(w_round1[5], 0x00000000,  "w_round1_5", 32);
    constantWithExist(w_round1[6], 0x00000000,  "w_round1_6", 32);
    constantWithExist(w_round1[7], 0x00000000,  "w_round1_7", 32);
    constantWithExist(w_round1[8], 0x00000000,  "w_round1_8", 32);
    constantWithExist(w_round1[9], 0x00000000,  "w_round1_9", 32);
    constantWithExist(w_round1[10], 0x00000000, "w_round1_10", 32);
    constantWithExist(w_round1[11], 0x00000000, "w_round1_11", 32);
    constantWithExist(w_round1[12], 0x00000000, "w_round1_12", 32);
    constantWithExist(w_round1[13], 0x00000000, "w_round1_13", 32);
    constantWithExist(w_round1[14], 0x00000000, "w_round1_14", 32);
    constantWithExist(w_round1[15], 0x00000280, "w_round1_15", 32);
}

void PowCnf::constantW_round2(){
#ifdef ROUND2
    constantWithExist(w_round2[8], 0x80000000,  "w_round2_8", 32);
    constantWithExist(w_round2[9], 0x00000000,  "w_round2_9", 32);
    constantWithExist(w_round2[10], 0x00000000, "w_round2_10", 32);
    constantWithExist(w_round2[11], 0x00000000, "w_round2_11", 32);
    constantWithExist(w_round2[12], 0x00000000, "w_round2_12", 32);
    constantWithExist(w_round2[13], 0x00000000, "w_round2_13", 32);
    constantWithExist(w_round2[14], 0x00000000, "w_round2_14", 32);
    constantWithExist(w_round2[15], 0x00000100, "w_round2_15", 32);
#endif 
}


void PowCnf::constantAToH(std::vector<uint32_t> first_round_output){
    // 初始化round1的A0到H0
    constantWithExist(A_round1[0], first_round_output[0], "A[0]_round1", 32);
    constantWithExist(B_round1[0], first_round_output[1], "B[0]_round1", 32);
    constantWithExist(C_round1[0], first_round_output[2], "C[0]_round1", 32);
    constantWithExist(D_round1[0], first_round_output[3], "D[0]_round1", 32);
    constantWithExist(E_round1[0], first_round_output[4], "E[0]_round1", 32);
    constantWithExist(F_round1[0], first_round_output[5], "F[0]_round1", 32);
    constantWithExist(G_round1[0], first_round_output[6], "G[0]_round1", 32);
    constantWithExist(H_round1[0], first_round_output[7], "H[0]_round1", 32);

#ifdef ROUND2
    constantWithExist(A_round2[0], a_to_h_init[0], "A[0]_round2", 32);
    constantWithExist(B_round2[0], a_to_h_init[1], "B[0]_round2", 32);
    constantWithExist(C_round2[0], a_to_h_init[2], "C[0]_round2", 32);
    constantWithExist(D_round2[0], a_to_h_init[3], "D[0]_round2", 32);
    constantWithExist(E_round2[0], a_to_h_init[4], "E[0]_round2", 32);
    constantWithExist(F_round2[0], a_to_h_init[5], "F[0]_round2", 32);
    constantWithExist(G_round2[0], a_to_h_init[6], "G[0]_round2", 32);
    constantWithExist(H_round2[0], a_to_h_init[7], "H[0]_round2", 32);
    // 初始化round2的A0到H0
#endif 
}

bool PowCnf::initVariables(){
    // 感觉这部分其实优化的空间其实很大
    // 可能有很多值是可以预计算的
    // todo：对一些常量进行直接赋值
    // 先计算sha256 forward 并作为第一轮的a0到h0
    
    // 初始化k的值
    constantK();

    // 初始化第一轮w0-w2, w4-w15的值
    constantW_round1();
    
    // 初始化第一轮a0到ah的值
    // 先将固定部分直接sha256_forward了

    // 直接memcpy第二轮中w0-w7的值，其实就是第一轮的a64-h64加a0-h64，结果就是w0到w7的序号
    constantW_round2();
    
    return true;
}

// clause

void PowCnf::rotl(int r[32], int x[32], unsigned int n){
    for (unsigned int i = 0; i < 32; ++i)
        r[i] = x[(i + 32 - n) % 32];
}

void PowCnf::fct_SHA256_SHR(int r[32], unsigned int bits, int word[32]){
    for (unsigned int i = 0; i < 32 - bits; i++)
        r[i] = word[i + bits];
    for (unsigned int i = 32 - bits; i < 32; ++i)
    {
        static bool first = true;
        static int t[1];

        if (first)
        {
            // r[i] = 0;
            // constant(r[i], 0);
            newVars("fillbit0", t, 1);
            constant(t, 0, "",  1);
            first = false;
        }
        r[i] = t[0];
    }
}

void PowCnf::fct_SHA256_ROTR(int r[32], unsigned int bits, int word[32]){
    for (unsigned int i = 0; i < 32; ++i)
        r[i] = word[(i + bits) % 32];
}

void PowCnf::fct_SHA256_SIGMA0(int r[32], int word[32]){
    int tmp1[32];
    // new_vars(format("tmp1 fct_SHA256_SIGMA0"), tmp1, 32);
    int tmp2[32];
    // new_vars(format("tmp2 fct_SHA256_SIGMA0"), tmp2, 32);
    int tmp3[32];
    // new_vars(format("tmp3 fct_SHA256_SIGMA0"), tmp3, 32);

    fct_SHA256_ROTR(tmp1, 2, word);
    fct_SHA256_ROTR(tmp2, 13, word);
    fct_SHA256_ROTR(tmp3, 22, word);

    xor3(r, tmp1, tmp2, tmp3, 32);
}

void PowCnf::fct_SHA256_SIGMA1(int r[32], int word[32]){
     int tmp1[32];
    // new_vars(format("tmp1 fct_SHA256_SIGMA1"), tmp1, 32);
    int tmp2[32];
    // new_vars(format("tmp2 fct_SHA256_SIGMA1"), tmp2, 32);
    int tmp3[32];
    // new_vars(format("tmp3 fct_SHA256_SIGMA1"), tmp3, 32);

    fct_SHA256_ROTR(tmp1, 6, word);
    fct_SHA256_ROTR(tmp2, 11, word);
    fct_SHA256_ROTR(tmp3, 25, word);

    xor3(r, tmp1, tmp2, tmp3, 32);
}

void PowCnf::fct_SHA256_sigma0(int r[32], int word[32]){
    int tmp1[32];
    // new_vars(format("tmp1 fct_SHA256_sigma0"), tmp1, 32);
    int tmp2[32];
    // new_vars(format("tmp2 fct_SHA256_sigma0"), tmp2, 32);
    int tmp3[32];
    // new_vars(format("tmp3 fct_SHA256_sigma0"), tmp3, 32);

    fct_SHA256_ROTR(tmp1, 7, word);
    fct_SHA256_ROTR(tmp2, 18, word);
    fct_SHA256_SHR(tmp3, 3, word);

    xor3(r, tmp1, tmp2, tmp3, 32);
}

void PowCnf::fct_SHA256_sigma1(int r[32], int word[32]){
    int tmp1[32];
    // new_vars(format("tmp1 fct_SHA256_sigma1"), tmp1, 32);
    int tmp2[32];
    // new_vars(format("tmp2 fct_SHA256_sigma1"), tmp2, 32);
    int tmp3[32];
    // new_vars(format("tmp3 fct_SHA256_sigma1"), tmp3, 32);

    fct_SHA256_ROTR(tmp1, 17, word);
    fct_SHA256_ROTR(tmp2, 19, word);
    fct_SHA256_SHR(tmp3, 10, word);

    xor3(r, tmp1, tmp2, tmp3, 32);
}

void PowCnf::fct_SHA_Ch(int r[32], int x[32], int y[32], int z[32], std::string help_text){
    int tmp1[32];
    std::string lable1 = "tmp1 fct_SHA_Ch_" + help_text;
    newVars(format(lable1.data()), tmp1, 32);
    int tmp2[32];
    std::string lable2 = "tmp1 fct_SHA_Ch_" + help_text;
    newVars(format(lable2.data()), tmp2, 32);

    xor2(tmp1, y, z, 32);
    and2(tmp2, x, tmp1, 32);
    xor2(r, tmp2, z, 32);
}

void PowCnf::fct_SHA_Maj(int r[32], int x[32], int y[32], int z[32]){
    int tmp1[32];
    newVars(format("tmp1 fct_SHA_Maj"), tmp1, 32);
    int tmp2[32];
    newVars(format("tmp2 fct_SHA_Maj"), tmp2, 32);
    int tmp3[32];
    newVars(format("tmp3 fct_SHA_Maj"), tmp3, 32);

    or2(tmp1, y, z, 32);
    and2(tmp2, x, tmp1, 32);
    and2(tmp3, y, z, 32);
    or2(r, tmp2, tmp3, 32);
}   


bool PowCnf::roundFuntion(){
    // todo:开始生成相关的clause
    // W的迭代有一半可以缩减一半子句， 因为有一半的全是0， 可以由add4变成add2,其实就变少了两个add2子句的数量
    // 
    // 计算w16-w63
    for (unsigned int i = 16; i < 64; i++)
    {
        // MM wt[i] = fct_SHA256_sigma1 (wt[i - 2]) + wt[i - 7] +
        // fct_SHA256_sigma0 (wt[i - 15]) + wt[i - 16];
        int tmp1_w_round1[32];
        newVars(format("tmp1 init_16_to_63 $", "round1"), tmp1_w_round1, 32);
        
        int tmp2_w_round1[32];
        newVars(format("tmp2 init_16_to_63 $", "round1"), tmp2_w_round1, 32);
        
        fct_SHA256_sigma1(tmp1_w_round1, w_round1[i - 2]);
        fct_SHA256_sigma0(tmp2_w_round1, w_round1[i - 15]);

        add4("init_16_to_63", w_round1[i], tmp1_w_round1, w_round1[i - 7], tmp2_w_round1, w_round1[i - 16]);

#ifdef ROUND2
 // DEBUG
        // round2
        int tmp1_w_round2[32];
        newVars(format("tmp1 init_16_to_63 $", "round2"), tmp1_w_round2, 32);

        int tmp2_w_round2[32];
        newVars(format("tmp2 init_16_to_63 $", "round2"), tmp2_w_round2, 32);

        fct_SHA256_sigma1(tmp1_w_round2, w_round2[i - 2]);
        fct_SHA256_sigma0(tmp2_w_round2, w_round2[i - 15]);
        
        add4("init_16_to_63", w_round2[i], tmp1_w_round2, w_round2[i - 7], tmp2_w_round2, w_round2[i - 16]);
#endif
    }

    // 计算轮函数的逻辑
    for (unsigned int t = 1; t <= 64; t++) // Round 0 is for init values!!!
    {
        // temp1 = H + SHA256_SIGMA1 (E) + SHA_Ch (E, F, G) + K[t] + W[t];

        fct_SHA256_SIGMA1(tmp1a_round1[t], E_round1[t - 1]);
        
        fct_SHA_Ch(tmp1b_round1[t], E_round1[t - 1], F_round1[t - 1], G_round1[t - 1], "round1_" + std::to_string(t));
        
        add5("caculateT1_round1_" + std::to_string(t), tmp1_round1[t], H_round1[t - 1], tmp1a_round1[t], tmp1b_round1[t], k[t - 1], w_round1[t - 1]);
        
        // temp2 = SHA256_SIGMA0 (A) + SHA_Maj (A, B, C);

        fct_SHA256_SIGMA0(tmp2a_round1[t], A_round1[t - 1]);
       

        fct_SHA_Maj(tmp2b_round1[t], A_round1[t - 1], B_round1[t - 1], C_round1[t - 1]);
        
        add2("caculateT2_round1_" + std::to_string(t), tmp2_round1[t], tmp2a_round1[t], tmp2b_round1[t]);
        
        // E = D + temp1;

        add2("caculateE_round1_" + std::to_string(t), E_round1[t], D_round1[t - 1], tmp1_round1[t]);

        // A = temp1 + temp2;

        add2("caculateA_round1_" + std::to_string(t), A_round1[t], tmp1_round1[t], tmp2_round1[t]);
        

#ifdef ROUND2
        fct_SHA256_SIGMA1(tmp1a_round2[t], E_round2[t - 1]);
        
        fct_SHA_Ch(tmp1b_round2[t], E_round2[t - 1], F_round2[t - 1], G_round2[t - 1], "round2_" + std::to_string(t));

        add5("caculateT1_round2_" + std::to_string(t), tmp1_round2[t], H_round2[t - 1], tmp1a_round2[t], tmp1b_round2[t], k[t - 1], w_round2[t - 1]);
        
        fct_SHA256_SIGMA0(tmp2a_round2[t], A_round2[t - 1]);

        fct_SHA_Maj(tmp2b_round2[t], A_round2[t - 1], B_round2[t - 1], C_round2[t - 1]);

        add2("caculateT2_round2_" + std::to_string(t), tmp2_round2[t], tmp2a_round2[t], tmp2b_round2[t]);
        
        add2("caculateE_round2_" + std::to_string(t), E_round2[t], D_round2[t - 1], tmp1_round2[t]);
        
        add2("caculateA_round1_" + std::to_string(t), A_round2[t], tmp1_round2[t], tmp2_round2[t]);
#endif
    }

    // 第一次的结果
    add2("w_round2_input", w_round2[0], A_round1[0], A_round1[64]);
    add2("w_round2_input", w_round2[1], B_round1[0], B_round1[64]);
    add2("w_round2_input", w_round2[2], C_round1[0], C_round1[64]);
    add2("w_round2_input", w_round2[3], D_round1[0], D_round1[64]);
    add2("w_round2_input", w_round2[4], E_round1[0], E_round1[64]);
    add2("w_round2_input", w_round2[5], F_round1[0], F_round1[64]);
    add2("w_round2_input", w_round2[6], G_round1[0], G_round1[64]);
    add2("w_round2_input", w_round2[7], H_round1[0], H_round1[64]);
    
    // 得出最终的结果
#ifdef ROUND2
 // DEBUG
    add2("h_out", h_out[0], A_round2[0], A_round2[64]);
    add2("h_out", h_out[1], B_round2[0], B_round2[64]);
    add2("h_out", h_out[2], C_round2[0], C_round2[64]);
    add2("h_out", h_out[3], D_round2[0], D_round2[64]);
    add2("h_out", h_out[4], E_round2[0], E_round2[64]);
    add2("h_out", h_out[5], F_round2[0], F_round2[64]);
    add2("h_out", h_out[6], G_round2[0], G_round2[64]);
    add2("h_out", h_out[7], H_round2[0], H_round2[64]);
#endif
    // 第一轮的结果由第二轮的W[0]-W[7]来接收

    return true;
}

// hout less than block target
void PowCnf::constrainHout(){
    int equal[25];
    int target_bit[24];
    int less[24];
    int pos_temp = 0;
   
    newVars("less_than|eqal_tmep", equal, 25);
    newVars("target_bits", target_bit, 24);
    newVars("less_result", less, 24);
    
    constant1(equal[0], true, "equal[0]");
    for(int i = 0; i < 32; i++){
        uint8_t temp = mining_block.target[i];
        // std::cout  << std::hex << std::setw(2) << std::setfill(' ') << static_cast<int>(temp) << " ";
        if(temp == 0){
            constant((&(h_out[7-(i/4)][8*(i%4)])), 0x00, "h_out", 8);
            continue;
        }

        // 数多少个位
        int last_pos = 31;
        for(; last_pos >= i && !mining_block.target[last_pos]; last_pos--);
        // std::cout << "the real interval == " << last_pos-i << std::endl;

        while(i <= last_pos){
            temp = mining_block.target[i];
            // std::cout  << std::hex << std::setw(2) << std::setfill(' ') << static_cast<int>(temp) << " ";
            for(int k = 0; k < 8; k++){
                bool left = temp & 0x80;
                // std::cout << "pos is" << std::endl;
                // std::cout<< std::dec << 7-(i/4) << " " << (24-8*(i%4)+k) <<" " << left << std::endl;
                constant1(target_bit[pos_temp], left, "");
                nxor2(&equal[pos_temp+1], &h_out[7-(i/4)][8*(i%4)+k], &target_bit[pos_temp], 1);
                lt2(&less[pos_temp], &h_out[7-(i/4)][8*(i%4)+k], &target_bit[pos_temp], &equal[pos_temp], 1);
                
                pos_temp++;
                temp <<= 1;
            }
            i++;
        }
        clause(less[0], less[1], less[2], less[3], less[4], less[5], less[6], less[7], 
               less[8], less[9], less[10], less[11], less[12], less[13], less[14], less[15], 
               less[16], less[17], less[18], less[19], less[20], less[21], less[22], less[23]);
        break;
    }
}

void PowCnf::setHout(std::vector<uint32_t> target){

    for(int i = 0; i < 8; i++){
        constant32(h_out[i], target[i], "h_out");
    }
}

void PowCnf::ConstrainNonce(uint32_t nonce, int left_bits){
    nonce_cnf.str("");
    nonce_clause_num = 16-left_bits;
    static bool tag = false;
    if(!tag){
        constant(&w_round1[3][8], 0x5d, "nonce_constant8", 8);
        constant(&w_round1[3][0], 0xab, "nonce_constant0", 8);
        tag = true;
    }

    switch (left_bits)
    {
    case 4:
        constant1(w_round1[3][27], 0, "w_round1_27");
        break;
    case 5:
        constant1(w_round1[3][26], 1, "w_round1_26");
        break;
    case 6:
        constant1(w_round1[3][25], 0, "w_round1_25");
        break;
    case 7:
        constant1(w_round1[3][24], 1, "w_round1_24");
        break;
    case 8:
        constant1(w_round1[3][23], 1, "w_round1_23");
        break;
    case 9:
        constant1(w_round1[3][22], 1, "w_round1_22");
        break;
    case 10:
        constant1(w_round1[3][21], 0, "w_round1_21");
        break;
    case 11:
        constant1(w_round1[3][20], 0, "w_round1_20");
        break;
    case 12:
        constant1(w_round1[3][19], 1, "w_round1_19");
        break;
    case 13:
        constant1(w_round1[3][18], 1, "w_round1_18");
        break;
    case 14:
        constant1(w_round1[3][17], 1, "w_round1_17");
        break;
    case 15:
        constant1(w_round1[3][16], 1, "w_round1_16");
        break;
        // case 32: constant(r0.w[3][0], 0);
    }
}


bool PowCnf::initBlockFromFile(std::string file_address){   

    if(!mining_block.createBlockJsonFromFile(file_address)){
        std::cerr << "create block from file failed" << std::endl;
        return false;
    }
    if(!mining_block.parseAllFromJsonBlock()){
        std::cerr << "parse config from json failed" << std::endl;
        return false;
    }
    if(!mining_block.createTargetFromBits()){
        std::cerr << "create target from bits failed" << std::endl;
        return false;
    }
    
    return true;
}

bool PowCnf::generateCnfFrameFromBlock(){

    if(!assignVariablesIndices()){
        std::cerr << "assignVariablesindices failed" << std::endl;
    }

    if(!initVariables()){
        std::cerr << "initVariables failed" << std::endl;
    }

    if(!roundFuntion()){
        std::cerr << "generate pow roundFunction failed" << std::endl;
    }

    return true;
}

void PowCnf::printMiningBlock(){
    outPutHex("vesion bytes", mining_block.version_bytes.data(), mining_block.version_bytes.size());
    outPutHex("previous hash bytes", mining_block.previous_block_hash_bytes.data(), mining_block.previous_block_hash_bytes.size());
    outPutHex("merkel root bytes", mining_block.merkel_root_bytes.data(), mining_block.merkel_root_bytes.size());
    outPutHex("time bytes", mining_block.time_bytes.data(), mining_block.time_bytes.size());
    outPutHex("bits bytes", mining_block.bits_bytes.data(), mining_block.bits_bytes.size());
}