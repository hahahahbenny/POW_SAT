#include "sha256.h"

std::string opensslSha256(const std::string str)
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

uint32_t SHA256_sigma0(uint32_t word)
{
    // return (SHA256_ROTR( 7,word) ^ SHA256_ROTR(18,word) ^ SHA256_SHR( 3,word));
    uint32_t tmp1;
    uint32_t tmp2;
    uint32_t tmp3;

    tmp1 = SHA256_ROTR(7, word);
    // printf("c SHA256_sigma0 tmp1=0x%08X\n", tmp1);
    tmp2 = SHA256_ROTR(18, word);
    // printf("c SHA256_sigma0 tmp2=0x%08X\n", tmp2);
    tmp3 = SHA256_SHR(3, word);
    // printf("c SHA256_sigma0 tmp3=0x%08X\n", tmp3);

    return tmp1 ^ tmp2 ^ tmp3;
}

uint32_t SHA256_sigma1(uint32_t word)
{
    // return (SHA256_ROTR(17,word) ^ SHA256_ROTR(19,word) ^ SHA256_SHR(10,word));
    uint32_t tmp1;
    uint32_t tmp2;
    uint32_t tmp3;

    tmp1 = SHA256_ROTR(17, word);
    // printf("c SHA256_sigma1 tmp1=0x%08X\n", tmp1);
    tmp2 = SHA256_ROTR(19, word);
    // printf("c SHA256_sigma1 tmp2=0x%08X\n", tmp2);
    tmp3 = SHA256_SHR(10, word);
    // printf("c SHA256_sigma1 tmp3=0x%08X\n", tmp3);

    return tmp1 ^ tmp2 ^ tmp3;
}

uint32_t rotl(uint32_t x, unsigned int n) { return (x << n) | (x >> (32 - n)); }

void sha256_forward(uint8_t Message_Block[64], int len, uint32_t h_out[8])
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

    A = h_out[0] = 0x6A09E667;
    B = h_out[1] = 0xBB67AE85;
    C = h_out[2] = 0x3C6EF372;
    D = h_out[3] = 0xA54FF53A;
    E = h_out[4] = 0x510E527F;
    F = h_out[5] = 0x9B05688C;
    G = h_out[6] = 0x1F83D9AB;
    H = h_out[7] = 0x5BE0CD19;

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