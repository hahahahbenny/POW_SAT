#include <openssl/sha.h>
#include <string>

// openssl sha256 Encapsulation
std::string opensslSha256(const std::string str);
/* Define the SHA shift, rotate left and rotate right macro */
#define SHA256_SHR(bits, word)  ((word) >> (bits))
#define SHA256_ROTL(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))
#define SHA256_ROTR(bits, word) (((word) >> (bits)) | ((word) << (32 - (bits))))

/* Define the SHA SIGMA and sigma macros */
#define SHA256_SIGMA0(word)     (SHA256_ROTR(2, word) ^ SHA256_ROTR(13, word) ^ SHA256_ROTR(22, word))
#define SHA256_SIGMA1(word)     (SHA256_ROTR(6, word) ^ SHA256_ROTR(11, word) ^ SHA256_ROTR(25, word))
#define  SHA_Ch(x, y, z)  (((x) & ((y) ^ (z))) ^ (z))
#define SHA_Maj(x, y, z) (((x) & ((y) | (z))) | ((y) & (z)))

uint32_t SHA256_sigma0(uint32_t word);

uint32_t SHA256_sigma1(uint32_t word);

uint32_t rotl(uint32_t x, unsigned int n);

void sha256_forward(uint8_t Message_Block[64], int len, uint32_t h_out[8]);

