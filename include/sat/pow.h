#ifndef POW
#define POW

#include "sat/cnf.h"
#include "block.h"
#include <cstdint>
#include <string>
#include <vector>
#include "sha256.h"

class PowCnf : public Cnf{
public:
    PowCnf(){};
    PowCnf(std::string block_address){
        initBlockFromFile(block_address);
    };
    bool initBlockFromFile(std::string file_address);
    bool generateCnfFrameFromBlock();
    void printMiningBlock();
    void constrainHout();
    void ConstrainNonce(uint32_t nonce, int left_bits);
    void setHout(std::vector<uint32_t> target);
    
private:
    // variables
    bool assignVariablesIndices();   
    bool initVariables();
    bool roundFuntion();
    void constantK();
    void constantW_round1();
    void constantW_round2();
    void constantAToH(std::vector<uint32_t> first_round_output);
    void constantWithExist(int r[], u_int32_t value, std::string help_text, int size);
    
    // clause
    void rotl(int r[32], int x[32], unsigned int n);
    void fct_SHA256_SHR(int r[32], unsigned int bits, int word[32]);
    void fct_SHA256_ROTR(int r[32], unsigned int bits, int word[32]);
    void fct_SHA256_SIGMA0(int r[32], int word[32]);
    void fct_SHA256_SIGMA1(int r[32], int word[32]);
    void fct_SHA256_sigma0(int r[32], int word[32]);
    void fct_SHA256_sigma1(int r[32], int word[32]);
    void fct_SHA_Ch(int r[32], int x[32], int y[32], int z[32], std::string help_text);
    void fct_SHA_Maj(int r[32], int x[32], int y[32], int z[32]);

    block mining_block;

    std::ostringstream nonce_cnf;
    int nonce_clause_num = 0;

    int k[64][32];
    int h_out[8][32]; //第一轮的输出直接接第二轮的输入，省一个等于的过程，那么此处就是第二轮的输出
    int constant_bool[2];
    
    uint32_t a_to_h_init[8] = {0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
                               0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};
    // round1
    int w_round1[64][32];
    int A_round1[1 + 64][32];
    int B_round1[1 + 64][32];
    int C_round1[1 + 64][32];
    int D_round1[1 + 64][32];
    int E_round1[1 + 64][32];
    int F_round1[1 + 64][32];
    int G_round1[1 + 64][32];
    int H_round1[1 + 64][32];

    int tmp1_round1[1 + 64][32];
    int tmp1a_round1[1 + 64][32];
    int tmp1b_round1[1 + 64][32];
    int tmp2_round1[1 + 64][32];
    int tmp2a_round1[1 + 64][32];
    int tmp2b_round1[1 + 64][32];

    // round2
    int w_round2[64][32];
    int A_round2[1 + 64][32];
    int B_round2[1 + 64][32];
    int C_round2[1 + 64][32];
    int D_round2[1 + 64][32];
    int E_round2[1 + 64][32];
    int F_round2[1 + 64][32];
    int G_round2[1 + 64][32];
    int H_round2[1 + 64][32];
    
    int tmp1_round2[1 + 64][32];
    int tmp1a_round2[1 + 64][32];
    int tmp1b_round2[1 + 64][32];
    int tmp2_round2[1 + 64][32];
    int tmp2a_round2[1 + 64][32];
    int tmp2b_round2[1 + 64][32];
    
};

#endif // !POW