#include <cassert>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <string>
#if defined _MSC_VER || defined _WIN32
#else
#include <cstdint>
#endif // defined _MSC_VER || defined _WIN32
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "block.h"
#include "sha256.h"

#include <boost/program_options.hpp>

extern "C"
{
#include <sys/types.h>
#if defined _MSC_VER || defined _WIN32
#else
#include <sys/wait.h>
#endif // defined _MSC_VER || defined _WIN32
#include <unistd.h>
}



#define SHA256_SHR(bits, word)  ((word) >> (bits))
#define SHA256_ROTL(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))
#define SHA256_ROTR(bits, word) (((word) >> (bits)) | ((word) << (32 - (bits))))

/* Define the SHA SIGMA and sigma macros */
#define SHA256_SIGMA0(word)     (SHA256_ROTR(2, word) ^ SHA256_ROTR(13, word) ^ SHA256_ROTR(22, word))
#define SHA256_SIGMA1(word)     (SHA256_ROTR(6, word) ^ SHA256_ROTR(11, word) ^ SHA256_ROTR(25, word))

#define SHA_Maj(x, y, z) (((x) & ((y) | (z))) | ((y) & (z)))
#define Maj_origin(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

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

#define SHA_Ch(x, y, z)  (((x) & ((y) ^ (z))) ^ (z))
#define SHA_Maj(x, y, z) (((x) & ((y) | (z))) | ((y) & (z)))


static void sha256_forward(uint8_t Message_Block[64], int len, uint32_t h_out[8], uint32_t last_h[8])
{
    /* Constants defined in FIPS-180-2, section 4.2.2 */
    static const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    int t, t4;                       /* Loop counter */
    uint32_t temp1, temp2;           /* Temporary word value */
    uint32_t temp1a, temp2a;         /* Temporary word value */
    uint32_t temp1b, temp2b;         /* Temporary word value */
    uint32_t W[64];                  /* Word sequence */
    uint32_t A, B, C, D, E, F, G, H; /* Word buffers */

    /*
     * Initialize the first 16 words in the array W
     */
    for (t = t4 = 0; t < 16; t++, t4 += 4)
        W[t] = (((uint32_t)Message_Block[t4]) << 24) | (((uint32_t)Message_Block[t4 + 1]) << 16) |
               (((uint32_t)Message_Block[t4 + 2]) << 8) | (((uint32_t)Message_Block[t4 + 3]));

    for (t = 16; t < 64; t++)
    {
#if 0
    printf("c Work on W[%u]\n", t);

    printf("c W[t - 2]=%08X\n", W[t - 2]);
    printf("c SHA256_sigma1 (W[t - 2])=%08X\n", SHA256_sigma1 (W[t - 2]));
    printf("c W[t - 7]=%08X\n", W[t - 7]);
    printf("c W[t - 15]=%08X\n", W[t - 15]);
    printf("c SHA256_sigma0 (W[t - 15])=%08X\n", SHA256_sigma0 (W[t - 15]));
    printf("c W[t - 16]=%08X\n", W[t - 16]);
#endif // 0

        W[t] = SHA256_sigma1(W[t - 2]) + W[t - 7] + SHA256_sigma0(W[t - 15]) + W[t - 16];
        // printf("c Work done on W[%u]\n", t);
    }

    // for (t = 0; t < 64; t++)
    // {
    //     printf("c W[%u]=0x%08X\n", t, W[t]);
    // }
    
    if(last_h != nullptr){
        A = h_out[0] = last_h[0];
        B = h_out[1] = last_h[1];
        C = h_out[2] = last_h[2];
        D = h_out[3] = last_h[3];
        E = h_out[4] = last_h[4];
        F = h_out[5] = last_h[5];
        G = h_out[6] = last_h[6];
        H = h_out[7] = last_h[7];
    }
    else{
        A = h_out[0] = 0x6A09E667;
        B = h_out[1] = 0xBB67AE85;
        C = h_out[2] = 0x3C6EF372;
        D = h_out[3] = 0xA54FF53A;
        E = h_out[4] = 0x510E527F;
        F = h_out[5] = 0x9B05688C;
        G = h_out[6] = 0x1F83D9AB;
        H = h_out[7] = 0x5BE0CD19;
    }
    

    for (t = 0; t < 64; t++)
    {
        // temp1 = H + SHA256_SIGMA1 (E) + SHA_Ch (E, F, G) + K[t] + W[t];
        temp1a = SHA256_SIGMA1(E);
        temp1b = SHA_Ch(E, F, G);
        temp1 = H + temp1a + temp1b + K[t] + W[t];
        // temp2 = SHA256_SIGMA0 (A) + SHA_Maj (A, B, C);
        temp2a = SHA256_SIGMA0(A);
        temp2b = SHA_Maj(A, B, C);
        temp2 = temp2a + temp2b;

        // printf("c Loop %u: temp1=%08X, temp1a=%08X, temp1b=%08X, temp2=%08X, "
        //        "temp2a=%08X, temp2b=%08X\n",
        //        t, temp1, temp1a, temp1b, temp2, temp2a, temp2b);

        H = G;
        G = F;
        F = E;
        E = D + temp1;
        D = C;
        C = B;
        B = A;
        A = temp1 + temp2;
    }

    h_out[0] += A;
    h_out[1] += B;
    h_out[2] += C;
    h_out[3] += D;
    h_out[4] += E;
    h_out[5] += F;
    h_out[6] += G;
    h_out[7] += H;
}

int main(){

    std::string file_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/resource/latestblock.json"; 
    // std::string cnf_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/bitcoin.cnf";

    // // open cnf file
    // std::ofstream cnf_file;
    // cnf_file.open(cnf_name);
    // if(! cnf_file.is_open()){
    //     std::cerr << "open file failed" << std::endl;
    // }

    // todo, 改进此处的输入和main函数， 保留tsetin编码和输出
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
    input_except_nonce += block_mining.nonce_bytes;

    if(!block_mining.createTargetFromBits()){
        std::cerr << "create from bits failed" << std::endl;
    }

    uint8_t input[64];
    memset(input, 0, 64);
    const uint8_t* input_block = reinterpret_cast<const unsigned char*>(input_except_nonce.data());
    for(int i = 0; i < 64; i++){
        input[i] = input_block[i];
    }

    std::vector<uint32_t> first_round_output(8, 0);
    std::vector<uint32_t> second_round_output(8, 0);
    std::vector<uint32_t> result(8, 0);

    sha_result = opensslSha256(input_except_nonce);
    sha_result = opensslSha256(sha_result);

    outPutHex("target", block_mining.target.data(), block_mining.target.size());

    for (uint8_t c : sha_result) {
    // 以十六进制形式输出每个字符的底层字节
        std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    std::cout << std::endl;

    // 直接先用sha256 forward模拟整个过程
    
    sha256_forward(input, 0, first_round_output.data(), nullptr);

    // std::cout << "the first round output is "<< std::endl;
    // std::cout << "----------------------------------------"<< std::endl;

    // for (uint32_t c : first_round_output) {
    // // 以十六进制形式输出每个字符的底层字节
    //     std::cout  << std::hex << std::setw(8) << std::setfill('0') << (c) << " ";
    // }
    // std::cout << std::endl;


// --------------------------------
    memset(input, 0, 64);
    for(int i = 0; i < 16; i++){
        input[i] = input_block[i+64];
    }

    input[16] = 0x80;
    input[62] =  0x02;
    input[63] = 0x80;
    sha256_forward(input, 0, second_round_output.data(), first_round_output.data());
    
    outPutHex("fisrt_round_output", first_round_output.data(), first_round_output.size());
  

    uint8_t* second_out = reinterpret_cast<uint8_t*>(second_round_output.data());

    // for (int i = 0; i < 32; i++) {
    // // 以十六进制形式输出每个字符的底层字节
    //     std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(second_out[i]) << " ";
    // }
    // std::cout << std::endl;

    // for (uint32_t c : second_round_output) {
    // // 以十六进制形式输出每个字符的底层字节
    //     std::cout  << std::hex << std::setw(8) << std::setfill('0') << (c) << " ";
    // }
    // std::cout << std::endl;

    // 最后一次
    memset(input, 0, 64);
    for(int i = 0; i < 32; i++){
        input[i] = second_out[((i/4)+1)*4 - i%4-1];
    }

    input[32] = 0x80;
    input[62] = 0x01;
    sha256_forward(input, 0, result.data(), nullptr);

    std::cout <<"the forward result is "  << std::endl;
    for (uint32_t c : second_round_output) {
    // 以十六进制形式输出每个字符的底层字节
        std::cout  << std::hex << std::setw(8) << std::setfill('0') << (c) << " ";
    }

    // 此处实际上是因为直接算的时候按uint32来放的，计算机里放的是小端法，所以每四个字节会反转。
    std::cout <<"\nthe forward result in second_out is"  << std::endl;
    for (int i = 0; i < 32; i++) {
    // 以十六进制形式输出每个字符的底层字节
        std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(second_out[i]) << " ";
    }


    std::cout <<"\nthe end result is "  << std::endl;
    for (uint32_t c : result) {
    // 以十六进制形式输出每个字符的底层字节
        std::cout  << std::hex << std::setw(8) << std::setfill('0') << (c) << " ";
    }
    std::cout << std::endl;

    return 0;
}