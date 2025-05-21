#include "sat/cnf.h"
#include "block.h"
#include <string>
#include "sha256.h"

class PowCnf : Cnf{
    PowCnf();
    bool initBlockFromFile(std::string file_address);
    bool generateCnfFromBlock();
    bool writeFile(std::string address) override;
    
private:
    bool initVariables();   
    bool roundFuntion();

    block input_block;
    int w[64][32];
    int h_out[8][32];
    
    uint32_t a_to_h_init[8] = {0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
                               0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};
    int A[1 + 64][32];
    int B[1 + 64][32];
    int C[1 + 64][32];
    int D[1 + 64][32];
    int E[1 + 64][32];
    int F[1 + 64][32];
    int G[1 + 64][32];
    int H[1 + 64][32];
    int tmp1[1 + 64][32];
    int tmp1a[1 + 64][32];
    int tmp1b[1 + 64][32];
    int tmp2[1 + 64][32];
    int tmp2a[1 + 64][32];
    int tmp2b[1 + 64][32];
    int k[64][32];
};