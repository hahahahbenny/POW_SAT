/*
 * sha256-sat -- SAT instance generator for SHA256
 * Copyright (C) 2011-2012  Vegard Nossum <vegard.nossum@gmail.com>
 * Copyright (C) 2013       Martin Maurer <martin.maurer@clibb.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ios>
#include <ostream>
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
#include <vector>
#include "block.h"

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

#include "sat/format.hh"

/********************* See RFC 4634 for details *********************/
/*
 * Description:
 *   This file implements the Secure Hash Signature Standard
 *   algorithms as defined in the National Institute of Standards
 *   and Technology Federal Information Processing Standards
 *   Publication (FIPS PUB) 180-1 published on April 17, 1995, 180-2
 *   published on August 1, 2002, and the FIPS PUB 180-2 Change
 *   Notice published on February 28, 2004.
 *
 *   A combined document showing all algorithms is available at
 *       http://csrc.nist.gov/publications/fips/
 *       fips180-2/fips180-2withchangenotice.pdf
 *
 *   The SHA-224 and SHA-256 algorithms produce 224-bit and 256-bit
 *   message digests for a given data stream. It should take about
 *   2**n steps to find a message with the same digest as a given
 *   message and 2**(n/2) to find any two messages with the same
 *   digest, when n is the digest size in bits. Therefore, this
 *   algorithm can serve as a means of providing a
 *   "fingerprint" for a message.
 *
 * Portability Issues:
 *   SHA-224 and SHA-256 are defined in terms of 32-bit "words".
 *   This code uses <stdint.h> (included via "sha.h") to define 32
 *   and 8 bit unsigned integer types. If your C compiler does not
 *   support 32 bit unsigned integers, this code is not
 *   appropriate.
 *
 * Caveats:
 *   SHA-224 and SHA-256 are designed to work with messages less
 *   than 2^64 bits long. This implementation uses SHA224/256Input()
 *   to hash the bits that are a multiple of the size of an 8-bit
 *   character, and then uses SHA224/256FinalBits() to hash the
 *   final few bits of the input.
 */

/* Taken over from sha225-256.c */

/* Define the SHA shift, rotate left and rotate right macro */
int ROUND = 64;
int LEFT_BITS = 0;

#define SHA256_SHR(bits, word)  ((word) >> (bits))
#define SHA256_ROTL(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))
#define SHA256_ROTR(bits, word) (((word) >> (bits)) | ((word) << (32 - (bits))))

/* Define the SHA SIGMA and sigma macros */
#define SHA256_SIGMA0(word)     (SHA256_ROTR(2, word) ^ SHA256_ROTR(13, word) ^ SHA256_ROTR(22, word))
#define SHA256_SIGMA1(word)     (SHA256_ROTR(6, word) ^ SHA256_ROTR(11, word) ^ SHA256_ROTR(25, word))

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



static uint32_t SHA256_sigma0(uint32_t word)
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

static uint32_t SHA256_sigma1(uint32_t word)
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

#define  SHA_Ch(x, y, z)  (((x) & ((y) ^ (z))) ^ (z))
#define SHA_Maj(x, y, z) (((x) & ((y) | (z))) | ((y) & (z)))

/* Instance options */
static std::string config_attack = "preimage";
static unsigned int config_nr_message_bits = 0;
static unsigned int config_nr_hash_bits = 256;

/* Format options */
static bool config_cnf = false;
static bool config_opb = false;

/* CNF options */
static bool config_use_xor_clauses = false;
static bool config_use_halfadder_clauses = false;
static bool config_use_tseitin_adders = false;
static bool config_restrict_branching = false;

/* OPB options */
static bool config_use_compact_adders = false;

static std::ostringstream cnf;
static std::ostringstream opb;

static void comment(std::string str)
{
    cnf << format("c $\n", str);
    opb << format("* $\n", str);
}

static int nr_variables = 0;
static int nr_variables_pre = 0;
static unsigned int nr_clauses = 0;
static unsigned int nr_xor_clauses = 0;
static unsigned int nr_constraints = 0;

static void new_vars(std::string label, int x[], unsigned int n, bool decision_var = true)
{
    for (unsigned int i = 0; i < n; ++i)
        x[i] = ++nr_variables;

    comment(format("var $/$ $", x[0], n, label));

    if (config_restrict_branching)
    {
        if (decision_var)
        {
            for (unsigned int i = 0; i < n; ++i)
                cnf << format("d $ 0\n", x[i]);
        }
        else
        {
            for (unsigned int i = 0; i < n; ++i)
                cnf << format("d -$ 0\n", x[i]);
        }
    }
}

static void constant(int r, bool value)
{
    cnf << format("$$ 0\n", value ? "" : "-", r);
    opb << format("1 x$ = $;\n", r, value ? 1 : 0);

    nr_clauses += 1;
    nr_constraints += 1;
}

static void constant32(int r[], uint32_t value, std::string helptext)
{
    comment(format("constant32 (#) -> $", value, helptext));

    for (unsigned int i = 0; i < 32; ++i)
    {
        cnf << format("$$ 0\n", (value >> i) & 1 ? "" : "-", r[i]);
        opb << format("1 x$ = $;\n", r[i], (value >> i) & 1);

        nr_clauses += 1;
        nr_constraints += 1;
    }
}

static void constant8(int r[], u_int8_t value){
    for (unsigned int i = 0; i < 8; ++i)
    {
        cnf << format("$$ 0\n", (value >> i) & 1 ? "" : "-", r[i]);

        nr_clauses += 1;
        nr_constraints += 1;
    }
}

static void new_constant(std::string label, int r[32], uint32_t value)
{
    new_vars(label, r, 32);
    constant32(r, value, label);
}

template <typename T> static void args_to_vector(std::vector<T> &v) {}

template <typename T, typename... Args> static void args_to_vector(std::vector<T> &v, T x, Args... args)
{
    v.push_back(x);
    return args_to_vector(v, args...);
}

static void clause(const std::vector<int> &v)
{
    for (int x : v)
    {
        cnf << format("$$ ", x < 0 ? "-" : "", abs(x));
        opb << format("1 $x$ ", x < 0 ? "~" : "", abs(x));
    }

    cnf << format("0\n");
    opb << format(">= 1;\n");

    nr_clauses += 1;
    nr_constraints += 1;
}

template <typename... Args> static void clause(Args... args)
{
    std::vector<int> v;
    args_to_vector(v, args...);
    clause(v);
}

static void xor_clause(const std::vector<int> &v)
{
    cnf << format("x ");

    for (int x : v)
        cnf << format("$$ ", x < 0 ? "-" : "", abs(x));

    cnf << format("0\n");

    nr_xor_clauses += 1;
}

template <typename... Args> static void xor_clause(Args... args)
{
    std::vector<int> v;
    args_to_vector(v, args...);
    xor_clause(v);
}

static void nxor2(int r[], int a[], int b[], unsigned int n){
    comment("nxor2");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < n; ++i)
            xor_clause(r[i], a[i], b[i]);
    }
    else{
        for (unsigned int i = 0; i < n; ++i)
        {
            clause(r[i], a[i], b[i]);
            for (unsigned int j = 0; j < 3; ++j)
            {   
                clause((j == 0) ? r[i] : -r[i], (j == 1) ? a[i] : -a[i], (j == 2) ? b[i] : -b[i]);
            }
        }
    }
}


static void xor2(int r[], int a[], int b[], unsigned int n)
{
    comment("xor2");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < n; ++i)
            xor_clause(-r[i], a[i], b[i]);
    }
    else
    {
        for (unsigned int i = 0; i < n; ++i)
        {
            for (unsigned int j = 0; j < 8; ++j)
            {
                if (__builtin_popcount(j ^ 1) % 2 == 1)
                    continue;

                clause((j & 1) ? -r[i] : r[i], (j & 2) ? a[i] : -a[i], (j & 4) ? b[i] : -b[i]);
            }
        }
    }
}

static void xor3(int r[], int a[], int b[], int c[], unsigned int n = 32)
{
    comment("xor3");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < n; ++i)
            xor_clause(-r[i], a[i], b[i], c[i]);
    }
    else
    {
        for (unsigned int i = 0; i < n; ++i)
        {
            for (unsigned int j = 0; j < 16; ++j)
            {
                if (__builtin_popcount(j ^ 1) % 2 == 0)
                    continue;

                clause((j & 1) ? -r[i] : r[i], (j & 2) ? a[i] : -a[i], (j & 4) ? b[i] : -b[i], (j & 8) ? c[i] : -c[i]);
            }
        }
    }
}

static void xor4(int r[32], int a[32], int b[32], int c[32], int d[32])
{
    comment("xor4");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < 32; ++i)
            xor_clause(-r[i], a[i], b[i], c[i], d[i]);
    }
    else
    {
        for (unsigned int i = 0; i < 32; ++i)
        {
            for (unsigned int j = 0; j < 32; ++j)
            {
                if (__builtin_popcount(j ^ 1) % 2 == 1)
                    continue;

                clause((j & 1) ? -r[i] : r[i], (j & 2) ? a[i] : -a[i], (j & 4) ? b[i] : -b[i], (j & 8) ? c[i] : -c[i],
                       (j & 16) ? d[i] : -d[i]);
            }
        }
    }
}

static void eq(int a[], int b[], unsigned int n = 32)
{
    comment("eq");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < n; ++i)
            xor_clause(-a[i], b[i]);
    }
    else
    {
        for (unsigned int i = 0; i < n; ++i)
        {
            clause(-a[i], b[i]);
            clause(a[i], -b[i]);
        }
    }
}

static void neq(int a[], int b[], unsigned int n = 32)
{
    comment("neq");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < n; ++i)
            xor_clause(a[i], b[i]);
    }
    else
    {
        for (unsigned int i = 0; i < n; ++i)
        {
            clause(a[i], b[i]);
            clause(-a[i], -b[i]);
        }
    }
}

static void and2(int r[], int a[], int b[], unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
    {
        clause(r[i], -a[i], -b[i]);
        clause(-r[i], a[i]);
        clause(-r[i], b[i]);
    }
}

static void or2(int r[], int a[], int b[], unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
    {
        clause(-r[i], a[i], b[i]);
        clause(r[i], -a[i]);
        clause(r[i], -b[i]);
    }
}


static void lt2(int r[], int a[], int b[], int equal[], unsigned int n) {
    comment("lt2");
    int temp[n];
    new_vars("lt", temp, n);
    for (unsigned int i = 0; i < n; ++i){
        clause(temp[i], a[i], -b[i]);
        and2(&r[i], &temp[i], &equal[i], 1);
    }
}

static void add2(std::string label, int r[32], int a[32], int b[32])
{
    comment("add2");

    if (config_use_tseitin_adders)
    {
        int c[31];
        new_vars("carry", c, 31);

        int t0[31];
        new_vars("t0", t0, 31);

        int t1[31];
        new_vars("t1", t1, 31);

        int t2[31];
        new_vars("t2", t2, 31);

        and2(c, a, b, 1);
        xor2(r, a, b, 1);

        xor2(t0, &a[1], &b[1], 31);
        and2(t1, &a[1], &b[1], 31);
        and2(t2, t0, c, 31);
        or2(&c[1], t1, t2, 30);
        xor2(&r[1], t0, c, 31);
    }
    else if (config_use_compact_adders)
    {
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, a[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, b[i]);

        for (unsigned int i = 0; i < 32; ++i)
            opb << format("-$ x$ ", 1UL << i, r[i]);

        opb << format("= 0;\n");

        ++nr_constraints;
    }
    // else
    // {
    //     std::vector<int> addends[32 + 5];
    //     for (unsigned int i = 0; i < 32; ++i)
    //     {
    //         addends[i].push_back(a[i]);
    //         addends[i].push_back(b[i]);

    //         unsigned int m = floor(log2(addends[i].size()));
    //         std::vector<int> rhs(1 + m);
    //         rhs[0] = r[i];
    //         new_vars(format("$_rhs[$]", label, i), &rhs[1], m);

    //         for (unsigned int j = 1; j < 1 + m; ++j)
    //             addends[i + j].push_back(rhs[j]);

    //         halfadder(addends[i], rhs);
    //     }
    // }
}

static void add4(std::string label, int r[32], int a[32], int b[32], int c[32], int d[32])
{
    comment("add4");

    if (config_use_tseitin_adders)
    {
        int t0[32];
        new_vars("t0", t0, 32);

        int t1[32];
        new_vars("t1", t1, 32);

        add2(label, t0, a, b);
        add2(label, t1, c, d);
        add2(label, r, t0, t1);
    }
    else if (config_use_compact_adders)
    {
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, a[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, b[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, c[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, d[i]);

        for (unsigned int i = 0; i < 32; ++i)
            opb << format("-$ x$ ", 1UL << i, r[i]);

        opb << format("= 0;\n");

        ++nr_constraints;
    }
    // else
    // {
    //     std::vector<int> addends[32 + 5];
    //     for (unsigned int i = 0; i < 32; ++i)
    //     {
    //         addends[i].push_back(a[i]);
    //         addends[i].push_back(b[i]);
    //         addends[i].push_back(c[i]);
    //         addends[i].push_back(d[i]);

    //         unsigned int m = floor(log2(addends[i].size()));
    //         std::vector<int> rhs(1 + m);
    //         rhs[0] = r[i];
    //         new_vars(format("$_rhs[$]", label, i), &rhs[1], m);

    //         for (unsigned int j = 1; j < 1 + m; ++j)
    //             addends[i + j].push_back(rhs[j]);

    //         halfadder(addends[i], rhs);
    //     }
    // }
}

static void add5(std::string label, int r[32], int a[32], int b[32], int c[32], int d[32], int e[32])
{
    comment("add5");

    if (config_use_tseitin_adders)
    {
        int t0[32];
        new_vars("t0", t0, 32);

        int t1[32];
        new_vars("t1", t1, 32);

        int t2[32];
        new_vars("t2", t2, 32);

        add2(label, t0, a, b);
        add2(label, t1, c, d);
        add2(label, t2, t0, t1);
        add2(label, r, t2, e);
    }
    else if (config_use_compact_adders)
    {
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, a[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, b[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, c[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, d[i]);
        for (unsigned int i = 0; i < 32; ++i)
            opb << format("$ x$ ", 1L << i, e[i]);

        for (unsigned int i = 0; i < 32; ++i)
            opb << format("-$ x$ ", 1UL << i, r[i]);

        opb << format("= 0;\n");

        ++nr_constraints;
    }
    // else
    // {
    //     std::vector<int> addends[32 + 5];
    //     for (unsigned int i = 0; i < 32; ++i)
    //     {
    //         addends[i].push_back(a[i]);
    //         addends[i].push_back(b[i]);
    //         addends[i].push_back(c[i]);
    //         addends[i].push_back(d[i]);
    //         addends[i].push_back(e[i]);

    //         unsigned int m = floor(log2(addends[i].size()));
    //         std::vector<int> rhs(1 + m);
    //         rhs[0] = r[i];
    //         new_vars(format("$_rhs[$]", label, i), &rhs[1], m);

    //         for (unsigned int j = 1; j < 1 + m; ++j)
    //             addends[i + j].push_back(rhs[j]);

    //         halfadder(addends[i], rhs);
    //     }
    // }
}

static void rotl(int r[32], int x[32], unsigned int n)
{
    for (unsigned int i = 0; i < 32; ++i)
        r[i] = x[(i + 32 - n) % 32];
}

static void fct_SHA256_SHR(int r[32], unsigned int bits, int word[32])
{
    // return word >> bits;
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
            new_vars("fillbit0", t, 1);
            constant(t[0], 0);
            first = false;
        }
        r[i] = t[0];
    }
}

static void fct_SHA256_ROTR(int r[32], unsigned int bits, int word[32])
{
    // return (((word) >> (bits)) | ((word) << (32-(bits))));
    for (unsigned int i = 0; i < 32; ++i)
        r[i] = word[(i + bits) % 32];
}

/* Define the SHA SIGMA and sigma macros as function */
static void fct_SHA256_SIGMA0(int r[32], int word[32])
{
    // return fct_SHA256_ROTR( 2,word) ^ fct_SHA256_ROTR(13,word) ^
    // fct_SHA256_ROTR(22,word);

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

static void fct_SHA256_SIGMA1(int r[32], int word[32])
{
    // return fct_SHA256_ROTR( 6,word) ^ fct_SHA256_ROTR(11,word) ^
    // fct_SHA256_ROTR(25,word);

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

static void fct_SHA256_sigma0(int r[32], int word[32])
{
    // return fct_SHA256_ROTR( 7,word) ^ fct_SHA256_ROTR(18,word) ^
    // fct_SHA256_SHR( 3,word);

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

static void fct_SHA256_sigma1(int r[32], int word[32])
{
    // return fct_SHA256_ROTR(17,word) ^ fct_SHA256_ROTR(19,word) ^
    // fct_SHA256_SHR(10,word);

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

static void fct_SHA_Ch(int r[32], int x[32], int y[32], int z[32])
{
    // return (x & (y ^ z)) ^ z;

    int tmp1[32];
    new_vars(format("tmp1 fct_SHA_Ch"), tmp1, 32);
    int tmp2[32];
    new_vars(format("tmp2 fct_SHA_Ch"), tmp2, 32);

    xor2(tmp1, y, z, 32);
    and2(tmp2, x, tmp1, 32);
    xor2(r, tmp2, z, 32);
}

static void fct_SHA_Maj(int r[32], int x[32], int y[32], int z[32])
{
    // return ((x & (y | z)) | (y & z));

    int tmp1[32];
    new_vars(format("tmp1 fct_SHA_Maj"), tmp1, 32);
    int tmp2[32];
    new_vars(format("tmp2 fct_SHA_Maj"), tmp2, 32);
    int tmp3[32];
    new_vars(format("tmp3 fct_SHA_Maj"), tmp3, 32);

    or2(tmp1, y, z, 32);
    and2(tmp2, x, tmp1, 32);
    and2(tmp3, y, z, 32);
    or2(r, tmp2, tmp3, 32);
}

class sha256
{
    // 此处需要加函数可以定义a0到h0以应对上一轮的输入，
    // 不然每一次sha256的初始化输入都是h0到h7
    // 方案：将a0到h0作为public的变量，进行初始化
    //

  public:
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
    std::string name;

    sha256(std::string name_input) : name(name_input) 
    {
        createSha256Cnf();
    }

    sha256(std::string name_input, std::vector<uint32_t> &lastRoundOutput) : name(name_input)
    {
        if (lastRoundOutput.size() == 8)
        {
            for (int i = 0; i < 8; i++)
            {
                a_to_h_init[i] = lastRoundOutput[i];
            }
        }
        createSha256Cnf();
    };
    
    bool createSha256Cnf(){
        if(name.empty()) return false;
        std::cout << "----------------------------------------------------------------------------------\n";
        iniConstant();
        
        roundFunction();
        // std::cout << "after round function 256\n";  
        // std::cout << "num of variables = " << nr_variables << "\nnum of clause = " << nr_clauses << "\n";
        ouputPreAddCur();
        // std::cout << "after output function 256\n";  
       
        std::cout << "----------------------------------------------------------------------------------\n";
        return true;
    }

    // 初始化相关的变量
    bool iniConstant()
    {   
        comment("sha256");
        for(unsigned int i = 0; i < 16; ++i){
            new_vars(format("w$[$]", name, i), w[i], 32, !config_restrict_branching);
        }
        std::cout <<std::setw(32) << "init w0-w15" << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
        nr_variables_pre = nr_variables;
        /* XXX: Fix this later by writing directly to w[i] */
        for (unsigned int i = 16; i < 64; ++i)
            new_vars(format("w$[$]", name, i), w[i], 32);
        std::cout <<std::setw(32) << "init w16-w63" << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
        nr_variables_pre = nr_variables;
        // std::cout << "after build w\n";  
        // std::cout << "num of variables = " << nr_variables << "\nnum of clause = " << nr_clauses << "\n";

        new_vars(format("h$_out0", name), h_out[0], 32);
        new_vars(format("h$_out1", name), h_out[1], 32);
        new_vars(format("h$_out2", name), h_out[2], 32);
        new_vars(format("h$_out3", name), h_out[3], 32);
        new_vars(format("h$_out4", name), h_out[4], 32);
        new_vars(format("h$_out5", name), h_out[5], 32);
        new_vars(format("h$_out6", name), h_out[6], 32);
        new_vars(format("h$_out7", name), h_out[7], 32);

        std::cout <<std::setw(32) << "init hout" << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
        nr_variables_pre = nr_variables;
        // std::cout << "after build hout \n";  
        // std::cout << "num of variables = " << nr_variables << "\nnum of clause = " << nr_clauses << "\n";

        for (unsigned int i = 16; i < 64; i++)
        {
            // MM wt[i] = fct_SHA256_sigma1 (wt[i - 2]) + wt[i - 7] +
            // fct_SHA256_sigma0 (wt[i - 15]) + wt[i - 16];

            int tmp1[32];
            new_vars(format("tmp1 init_16_to_63 $", name), tmp1, 32);
            int tmp2[32];
            new_vars(format("tmp2 init_16_to_63 $", name), tmp2, 32);

            fct_SHA256_sigma1(tmp1, w[i - 2]);
            fct_SHA256_sigma0(tmp2, w[i - 15]);
            
            // std::cout << "after compute w with sigma0and1" << i << "value(sigma0 and sigma1)\n";  
            // std::cout << "num of variables = " << nr_variables << "\nnum of clause = " << nr_clauses << "\n\n";

            add4("init_16_to_63", w[i], tmp1, w[i - 7], tmp2, w[i - 16]);

            // std::cout << "after compute w with add4" << i << "value(sigma0 and sigma1)\n";  
            // std::cout << "num of variables = " << nr_variables << "\nnum of clause = " << nr_clauses << "\n\n";
        }
        std::cout <<std::setw(32) << "caculate w16-w63" << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
        nr_variables_pre = nr_variables;

        // std::cout << "after compute wi value(sigma0 and sigma1)\n";  
        // std::cout << "num of variables = " << nr_variables << "\nnum of clause = " << nr_clauses << "\n";

        /* Fix constants */

        new_constant("k[ 0]", k[0], 0x428a2f98);
        new_constant("k[ 1]", k[1], 0x71374491);
        new_constant("k[ 2]", k[2], 0xb5c0fbcf);
        new_constant("k[ 3]", k[3], 0xe9b5dba5);
        new_constant("k[ 4]", k[4], 0x3956c25b);

        new_constant("k[ 5]", k[5], 0x59f111f1);
        new_constant("k[ 6]", k[6], 0x923f82a4);
        new_constant("k[ 7]", k[7], 0xab1c5ed5);
        new_constant("k[ 8]", k[8], 0xd807aa98);
        new_constant("k[ 9]", k[9], 0x12835b01);

        new_constant("k[10]", k[10], 0x243185be);
        new_constant("k[11]", k[11], 0x550c7dc3);
        new_constant("k[12]", k[12], 0x72be5d74);
        new_constant("k[13]", k[13], 0x80deb1fe);
        new_constant("k[14]", k[14], 0x9bdc06a7);

        new_constant("k[15]", k[15], 0xc19bf174);
        new_constant("k[16]", k[16], 0xe49b69c1);
        new_constant("k[17]", k[17], 0xefbe4786);
        new_constant("k[18]", k[18], 0x0fc19dc6);
        new_constant("k[19]", k[19], 0x240ca1cc);

        new_constant("k[20]", k[20], 0x2de92c6f);
        new_constant("k[21]", k[21], 0x4a7484aa);
        new_constant("k[22]", k[22], 0x5cb0a9dc);
        new_constant("k[23]", k[23], 0x76f988da);
        new_constant("k[24]", k[24], 0x983e5152);

        new_constant("k[25]", k[25], 0xa831c66d);
        new_constant("k[26]", k[26], 0xb00327c8);
        new_constant("k[27]", k[27], 0xbf597fc7);
        new_constant("k[28]", k[28], 0xc6e00bf3);
        new_constant("k[29]", k[29], 0xd5a79147);

        new_constant("k[30]", k[30], 0x06ca6351);
        new_constant("k[31]", k[31], 0x14292967);
        new_constant("k[32]", k[32], 0x27b70a85);
        new_constant("k[33]", k[33], 0x2e1b2138);
        new_constant("k[34]", k[34], 0x4d2c6dfc);

        new_constant("k[35]", k[35], 0x53380d13);
        new_constant("k[36]", k[36], 0x650a7354);
        new_constant("k[37]", k[37], 0x766a0abb);
        new_constant("k[38]", k[38], 0x81c2c92e);
        new_constant("k[39]", k[39], 0x92722c85);

        new_constant("k[40]", k[40], 0xa2bfe8a1);
        new_constant("k[41]", k[41], 0xa81a664b);
        new_constant("k[42]", k[42], 0xc24b8b70);
        new_constant("k[43]", k[43], 0xc76c51a3);
        new_constant("k[44]", k[44], 0xd192e819);

        new_constant("k[45]", k[45], 0xd6990624);
        new_constant("k[46]", k[46], 0xf40e3585);
        new_constant("k[47]", k[47], 0x106aa070);
        new_constant("k[48]", k[48], 0x19a4c116);
        new_constant("k[49]", k[49], 0x1e376c08);

        new_constant("k[50]", k[50], 0x2748774c);
        new_constant("k[51]", k[51], 0x34b0bcb5);
        new_constant("k[52]", k[52], 0x391c0cb3);
        new_constant("k[53]", k[53], 0x4ed8aa4a);
        new_constant("k[54]", k[54], 0x5b9cca4f);

        new_constant("k[55]", k[55], 0x682e6ff3);
        new_constant("k[56]", k[56], 0x748f82ee);
        new_constant("k[57]", k[57], 0x78a5636f);
        new_constant("k[58]", k[58], 0x84c87814);
        new_constant("k[59]", k[59], 0x8cc70208);

        new_constant("k[60]", k[60], 0x90befffa);
        new_constant("k[61]", k[61], 0xa4506ceb);
        new_constant("k[62]", k[62], 0xbef9a3f7);
        new_constant("k[63]", k[63], 0xc67178f2);

        std::cout <<std::setw(32) << "init k0-k63" << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
        nr_variables_pre = nr_variables;

        // allocate the varians number to sat
        for (unsigned int t = 0; t <= 64; t++)
        {
            if (t > 0)
            {
                new_vars(format("tmp1$[$]", name, t), tmp1[t], 32);
                new_vars(format("tmp1a$[$]", name, t), tmp1a[t], 32);
                new_vars(format("tmp1b$[$]", name, t), tmp1b[t], 32);
                new_vars(format("tmp2$[$]", name, t), tmp2[t], 32);
                new_vars(format("tmp2a$[$]", name, t), tmp2a[t], 32);
                new_vars(format("tmp2b$[$]", name, t), tmp2b[t], 32);
            }
            new_vars(format("A$[$]", name, t), A[t], 32);
            new_vars(format("B$[$]", name, t), B[t], 32);
            new_vars(format("C$[$]", name, t), C[t], 32);
            new_vars(format("D$[$]", name, t), D[t], 32);
            new_vars(format("E$[$]", name, t), E[t], 32);
            new_vars(format("F$[$]", name, t), F[t], 32);
            new_vars(format("G$[$]", name, t), G[t], 32);
            new_vars(format("H$[$]", name, t), H[t], 32);
        }
        
        std::cout <<std::setw(32) << "init a0h0-a64h64 and temp12ab" << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
        nr_variables_pre = nr_variables;
        // std::cout << "after build AtoH and its between variables\n";  
        // std::cout << "num of variables = " << nr_variables << "\nnum of clause = " << nr_clauses << "\n";


        // init a0 to h0
        constant32(A[0], a_to_h_init[0], "A[0]");
        constant32(B[0], a_to_h_init[1], "B[0]");
        constant32(C[0], a_to_h_init[2], "C[0]");
        constant32(D[0], a_to_h_init[3], "D[0]");
        constant32(E[0], a_to_h_init[4], "E[0]");
        constant32(F[0], a_to_h_init[5], "F[0]");
        constant32(G[0], a_to_h_init[6], "G[0]");
        constant32(H[0], a_to_h_init[7], "H[0]");


        return true;
    };

    // 轮函数
    bool roundFunction(){
        for (unsigned int t = 1; t <= ROUND; t++) // Round 0 is for init values!!!
        {
            // temp1 = H + SHA256_SIGMA1 (E) + SHA_Ch (E, F, G) + K[t] + W[t];

            fct_SHA256_SIGMA1(tmp1a[t], E[t - 1]);

            std::string round_label = "round_" + std::to_string(t) + "_SIGMA1";
            std::cout <<std::setw(32) << round_label << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
            nr_variables_pre = nr_variables;

            fct_SHA_Ch(tmp1b[t], E[t - 1], F[t - 1], G[t - 1]);
            
            round_label = "round_" + std::to_string(t) + "_ch";
            std::cout <<std::setw(32) << round_label << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
            nr_variables_pre = nr_variables;
            
            add5("mainloop1", tmp1[t], H[t - 1], tmp1a[t], tmp1b[t], k[t - 1], w[t - 1]);
            
            round_label = "round_"  + std::to_string(t) + "_caculateT1";
            std::cout <<std::setw(32) << round_label << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
            nr_variables_pre = nr_variables;
            // temp2 = SHA256_SIGMA0 (A) + SHA_Maj (A, B, C);

            fct_SHA256_SIGMA0(tmp2a[t], A[t - 1]);

            round_label = "round_"  + std::to_string(t) + "_SIGMA0";
            std::cout <<std::setw(32) << round_label << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
            nr_variables_pre = nr_variables;
            fct_SHA_Maj(tmp2b[t], A[t - 1], B[t - 1], C[t - 1]);

            round_label = "round_"  + std::to_string(t) + "_MAJ";
            std::cout <<std::setw(32) << round_label << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
            nr_variables_pre = nr_variables;

            add2("mainloop2", tmp2[t], tmp2a[t], tmp2b[t]);


            round_label = "round_"  + std::to_string(t) + "_caculateT2";
            std::cout <<std::setw(32) << round_label << std::setw(32) << nr_variables <<std::setw(32) << nr_variables-nr_variables_pre << "\n";
            nr_variables_pre = nr_variables;

            // H = G;

            eq(H[t], G[t - 1]);
            // constant32(H[t], t, "H[t]");

            // G = F;

            eq(G[t], F[t - 1]);

            // F = E;

            eq(F[t], E[t - 1]);

            // E = D + temp1;

            add2("mainloop3", E[t], D[t - 1], tmp1[t]);

            // D = C;

            eq(D[t], C[t - 1]);

            // C = B;

            eq(C[t], B[t - 1]);

            // B = A;

            eq(B[t], A[t - 1]);

            // A = temp1 + temp2;

            add2("mainloop4", A[t], tmp1[t], tmp2[t]);
        }

        return true;
    };

    // 添加i-1次的H0-H7
    bool ouputPreAddCur(){
        add2("h_out", h_out[0], A[0], A[64]);
        add2("h_out", h_out[1], B[0], B[64]);
        add2("h_out", h_out[2], C[0], C[64]);
        add2("h_out", h_out[3], D[0], D[64]);
        add2("h_out", h_out[4], E[0], E[64]);
        add2("h_out", h_out[5], F[0], F[64]);
        add2("h_out", h_out[6], G[0], G[64]);
        add2("h_out", h_out[7], H[0], H[64]);
        return true;
    };

};

static uint32_t rotl(uint32_t x, unsigned int n) { return (x << n) | (x >> (32 - n)); }

static void sha256_forward(uint8_t Message_Block[64], int len, uint32_t h_out[8])
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

static void preimage()
{
    sha256 f("");

#if 0  // MM
  /* Generate a known-valid (messsage, hash)-pair */
  uint32_t w[64];
  for (unsigned int i = 0; i < 16; ++i)
    w[i] = lrand48();
#endif // MM

#if 0  // MM
  for (unsigned int i = 0; i < 16; ++i)
    w[i] = 0x41414141 + i + i << 8 + i << 16 + i << 24;
#endif // MM

    // MM std::cout << "c Here would be assignment of w[i]" << std::endl;

#if 0
  uint32_t h[8];
  sha256_forward((uint8_t *)w, 64, h);

  std::cout << "c After sha256_forward" << std::endl;

  /* Fix message bits */
  comment(format("Fix $ message bits", config_nr_message_bits));

  std::vector<unsigned int> message_bits(512);
  for (unsigned int i = 0; i < 512; ++i)
    message_bits[i] = i;

  // MM std::random_shuffle(message_bits.begin(), message_bits.end());
  for (unsigned int i = 0; i < config_nr_message_bits; ++i) {
    unsigned int r = message_bits[i] / 32;
    unsigned int s = message_bits[i] % 32;

    constant(f.w[r][s], (w[r] >> s) & 1);
  }

  /* Fix hash bits */
  comment(format("Fix $ hash bits", config_nr_hash_bits));

  std::vector<unsigned int> hash_bits(256);
  for (unsigned int i = 0; i < 256; ++i)
    hash_bits[i] = i;

  // MM std::random_shuffle(hash_bits.begin(), hash_bits.end());
  for (unsigned int i = 0; i < config_nr_hash_bits; ++i) {
    unsigned int r = hash_bits[i] / 32;
    unsigned int s = hash_bits[i] % 32;

    constant(f.h_out[r][s], (h[r] >> s) & 1);
  }
#endif // 0
}

/* The second preimage differs from the first preimage by flipping one of
 * the message bits. */
static void second_preimage()
{
    sha256 f("");

#if 0  // MM
  /* Generate a known-valid (messsage, hash)-pair */
  uint32_t w[80];
  for (unsigned int i = 0; i < 16; ++i)
    w[i] = lrand48();

  uint32_t h[5];
  sha256_forward(w, h);

  /* Fix message bits */
  comment(format("Fix $ message bits", config_nr_message_bits));

  std::vector<unsigned int> message_bits(512);
  for (unsigned int i = 0; i < 512; ++i)
    message_bits[i] = i;

  // MM std::random_shuffle(message_bits.begin(), message_bits.end());

  /* Flip the first bit */
  if (config_nr_message_bits > 0) {
    unsigned int r = message_bits[0] / 32;
    unsigned int s = message_bits[0] % 32;

    constant(f.w[r][s], !((w[r] >> s) & 1));
  }

  for (unsigned int i = 1; i < config_nr_message_bits; ++i) {
    unsigned int r = message_bits[i] / 32;
    unsigned int s = message_bits[i] % 32;

    constant(f.w[r][s], (w[r] >> s) & 1);
  }

  /* Fix hash bits */
  comment(format("Fix $ hash bits", config_nr_hash_bits));

  std::vector<unsigned int> hash_bits(160);
  for (unsigned int i = 0; i < 160; ++i)
    hash_bits[i] = i;

  // MM std::random_shuffle(hash_bits.begin(), hash_bits.end());
  for (unsigned int i = 0; i < config_nr_hash_bits; ++i) {
    unsigned int r = hash_bits[i] / 32;
    unsigned int s = hash_bits[i] % 32;

    constant(f.h_out[r][s], (h[r] >> s) & 1);
  }
#endif // 0 MM
}

static void collision()
{
    sha256 f("0");
    sha256 g("1");

    if (config_nr_message_bits > 0)
        std::cerr << "warning: collision attacks do not use fixed message bits\n";

    /* Fix message bits (set m != m') */
    comment(format("Fix $ message bits", config_nr_message_bits));

    std::vector<unsigned int> message_bits(512);
    for (unsigned int i = 0; i < 512; ++i)
        message_bits[i] = i;

    // MM std::random_shuffle(message_bits.begin(), message_bits.end());

    /* Flip some random bit */
    {
        unsigned int r = message_bits[0] / 32;
        unsigned int s = message_bits[0] % 32;

        neq(&f.w[r][s], &g.w[r][s], 1);
    }

    /* Fix hash bits (set H = H') */
    comment(format("Fix $ hash bits", config_nr_hash_bits));

    std::vector<unsigned int> hash_bits(160);
    for (unsigned int i = 0; i < 160; ++i)
        hash_bits[i] = i;

    // MM std::random_shuffle(hash_bits.begin(), hash_bits.end());
    for (unsigned int i = 0; i < config_nr_hash_bits; ++i)
    {
        unsigned int r = hash_bits[i] / 32;
        unsigned int s = hash_bits[i] % 32;

        eq(&f.h_out[r][s], &g.h_out[r][s], 1);
    }
}

static bool bitcoin(std::string block_without_nonce, const uint8_t* target)
{
    // 此处没有nonce的就是76字节
    if(block_without_nonce.size() != 76) return false;    
    // todo
    uint8_t input[64];
    const uint32_t* input_block = reinterpret_cast<const unsigned int*>(block_without_nonce.data());
    for(int i = 0; i < 64; i++){
        input[i] = block_without_nonce[i];
    }
    std::vector<uint32_t> first_round_output(8, 0);

    sha256_forward(input, 0, first_round_output.data());

    outPutHex("fisrt_1block_output", first_round_output.data(), first_round_output.size());

    std::cout << std::setfill(' ');
    std::cout << std::left << std::setw(32) << "after_function" << std::setw(32) << "num_of_variable" <<std::setw(32) << "add_variables" << "\n";
    sha256 r0("r0", first_round_output);
    
    // 此处不清楚直接这样处理会不会有问题，推测直接用first round output进行赋值没问题，后面的就不一定，我感觉大概率是输入有问题

    // 限制r0的输入w0到w15
    for(int i = 0; i < 12; i++){
        constant8(&(r0.w[i/4][24-(8*(i%4))]), block_without_nonce[i+64]);
    }

    // 就是这里很容易出错，就是nonce应该是小端法来存的，而不是直接用uint32的形式来存
    // 0x65cf5dab
    switch(LEFT_BITS){
        case 4: constant(r0.w[3][27], 0);
        case 5: constant(r0.w[3][26], 1);
        case 6: constant(r0.w[3][25], 0);
        case 7: constant(r0.w[3][24], 1);
        case 8: constant(r0.w[3][23], 1);
        case 9: constant(r0.w[3][22], 1);
        case 10: constant(r0.w[3][21], 0);
        case 11: constant(r0.w[3][20], 0);
        case 12: constant(r0.w[3][19], 1);
        case 13: constant(r0.w[3][18], 1);
        case 14: constant(r0.w[3][17], 1);
        case 15: constant(r0.w[3][16], 1);
        
        
        // case 32: constant(r0.w[3][0], 0);
        default:    constant8(&r0.w[3][8], 0x5d);
                    constant8(&r0.w[3][0], 0xab);
    }

    // constant32(r0.w[3], 0x05e0ed6d, "nonce"); 
    // 此处是block2的nonce
    // constant32(r0.w[3], 0x08d2bd61, "nonce"); 
    // 此处是最新的区块nonce85930942
    // constant32( r0.w[3], 0x65cf5dab, "nonce");
    // constant8(&r0.w[3][0], 0x61);
    // constant8(&r0.w[3][8], 0xbd);
    // constant8(&r0.w[3][16], 0xd2);
    // constant(r0.w[3][24], 0);
    // constant(r0.w[3][25], 0);
    // constant(r0.w[3][26], 0);
    // constant(r0.w[3][27], 1);
    // constant(r0.w[3][8], 1);
    // constant(r0.w[3][9], 0);
    // constant(r0.w[3][10], 1);
    // constant(r0.w[3][11], 1);
    // constant(r0.w[3][12], 1);
    // constant(r0.w[3][13], 1);
    // constant(r0.w[3][14], 0);
    
    // constant(r0.w[3][16], 0);
    // constant(r0.w[3][17], 1);
    // constant(r0.w[3][18], 0);
    // constant(r0.w[3][19], 0);
    // constant(r0.w[3][20], 1);
    // constant(r0.w[3][21], 0);
    // constant(r0.w[3][22], 1);
    // constant8(&r0.w[3][24], 0xfb);

    // constant(r0.w[3][27], 1);
    
    constant32(r0.w[4], 0x80000000, "MSG_R1_W4");
    constant32(r0.w[5], 0x00000000, "MSG_R1_W5");
    constant32(r0.w[6], 0x00000000, "MSG_R1_W6");
    constant32(r0.w[7], 0x00000000, "MSG_R1_W7");
    constant32(r0.w[8], 0x00000000, "MSG_R1_W8");
    constant32(r0.w[9], 0x00000000, "MSG_R1_W9");
    constant32(r0.w[10], 0x00000000, "MSG_R1_W10");
    constant32(r0.w[11], 0x00000000, "MSG_R1_W11");
    constant32(r0.w[12], 0x00000000, "MSG_R1_W12");
    constant32(r0.w[13], 0x00000000, "MSG_R1_W13");
    constant32(r0.w[14], 0x00000000, "MSG_R1_W14");
    constant32(r0.w[15], 0x00000280, "MSG_R1_W15");

    // 后面是在限制r1的 输入，因为上一轮只有256位输入，还要添加输出
    sha256 r1("r1");
    // 此处应该是三轮，但是实际上我可以将r0的输入改为前面区块的输出，就直接得到相应的结果了，所以要改的只有两个，一个是修改输入的初始值，第二个是完成less_than逻辑
    // 此处直接先把一轮的结果得到作为第二轮的a和h,
    // 说明第二轮中起码得暴露a[0]到h[0]

    constant32(&r1.w[8][0], 0x80000000, "MSG_R1_W8");
    constant32(&r1.w[9][0], 0x00000000, "MSG_R1_W9");
    constant32(&r1.w[10][0], 0x00000000, "MSG_R1_W10");
    constant32(&r1.w[11][0], 0x00000000, "MSG_R1_W11");
    constant32(&r1.w[12][0], 0x00000000, "MSG_R1_W12");
    constant32(&r1.w[13][0], 0x00000000, "MSG_R1_W13");
    constant32(&r1.w[14][0], 0x00000000, "MSG_R1_W14");
    constant32(&r1.w[15][0], 0x00000100, "MSG_R1_W15");
    // 哦哦它这里实际上是将r0的输出当成了直接对头输入的结果了
    // r2 就是第二次hash然后要满足target小于某个值的定义了
    /* Fill output of round 0 (r0) into input of round 1 (r1) */
    // 说明这里的h_out没啥问题，
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            eq(&r1.w[i][j], &r0.h_out[i][j], 1);
        }
    }
    

    // 此处定义equal的中间变量以便后面使用
    int equal[25];
    int target_bit[24];
    int less[24];
    int pos_temp = 0;
   
    new_vars("less_than|eqal_tmep", equal, 25);
    new_vars("target_bits", target_bit, 24);
    new_vars("less_result", less, 24);
    
    constant(equal[0], true);
    for(int i = 0; i < 32; i++){
        uint8_t temp = target[i];
        // std::cout  << std::hex << std::setw(2) << std::setfill(' ') << static_cast<int>(temp) << " ";
        if(temp == 0){
            constant8((&(r1.h_out[7-(i/4)][8*(i%4)])), 0x00);
            continue;
        }

        // 数多少个位
        int last_pos = 31;
        for(; last_pos >= i && !target[last_pos]; last_pos--);
        // std::cout << "the real interval == " << last_pos-i << std::endl;

        while(i <= last_pos){
            temp = target[i];
            // std::cout  << std::hex << std::setw(2) << std::setfill(' ') << static_cast<int>(temp) << " ";
            for(int k = 0; k < 8; k++){
                bool left = temp & 0x80;
                // std::cout << "pos is" << std::endl;
                // std::cout<< std::dec << 7-(i/4) << " " << (24-8*(i%4)+k) <<" " << left << std::endl;
                constant(target_bit[pos_temp], left);
                nxor2(&equal[pos_temp+1], &r1.h_out[7-(i/4)][8*(i%4)+k], &target_bit[pos_temp], 1);
                lt2(&less[pos_temp], &r1.h_out[7-(i/4)][8*(i%4)+k], &target_bit[pos_temp], &equal[pos_temp], 1);
                
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

// 直接赋hash值进行测试


    // constant32(&r1.h_out[0][0], 0xc7437a17, "MSG_R1_W8");
    // constant32(&r1.h_out[1][0], 0x43fbcfe3, "MSG_R1_W9");
    // constant32(&r1.h_out[2][0], 0x9f295d24, "MSG_R1_W10");
    // constant32(&r1.h_out[3][0], 0x54eb7063, "MSG_R1_W11");
    // constant32(&r1.h_out[4][0], 0xcc96d219, "MSG_R1_W12");
    // constant32(&r1.h_out[5][0], 0x944d0100, "MSG_R1_W13");
    // constant32(&r1.h_out[6][0], 0x00000000, "MSG_R1_W14");
    // constant32(&r1.h_out[7][0], 0x00000000, "MSG_R1_W15");

    return true;
}

// 2. 测试进行forward以后的sha256是否可以满足
void testSatUnit(std::string block_without_nonce){
    uint8_t input[64];
    const uint32_t* input_block = reinterpret_cast<const unsigned int*>(block_without_nonce.data());
    for(int i = 0; i < 64; i++){
        input[i] = block_without_nonce[i];
    }
    std::vector<uint32_t> W(8, 0);

    // for (t = t4 = 0; t < 16; t++, t4 += 4)
    //     W[t] = (((uint32_t)block_without_nonce[t4]) << 24) | (((uint32_t)block_without_nonce[t4 + 1]) << 16) |
    //            (((uint32_t)block_without_nonce[t4 + 2]) << 8) | (((uint32_t)block_without_nonce[t4 + 3]));

    std::vector<uint32_t> target = {0xbddd99cc, 0xfda39da1, 0xb108ce1a, 0x5d70038d, 
                                    0x0a967bac, 0xb68b6b63, 0x065f626a, 0x00000000};

    std::vector<uint32_t> last = {0x9cbba918, 0x50f7347b, 0xc2f08d32, 0x5af67165,
                                  0xf77751ad, 0x85f909c8, 0x5ae10340, 0xa3165b69};
 
    sha256 r0("r0");
    
    // 限制输入
    // for(int i = 0; i < 12; i++){
    //     constant8(&(r0.w[i/4][24-(8*(i%4))]), block_without_nonce[i+64]);
    // }

    for(int i = 0; i < 8; i++){
        constant32(&(r0.w[i][0]), last[i], "constriant w0-w7");
    }

    // constant32(&r0.w[3][0], 0x08d2bd61,"nonce_constraint");
    constant32(&r0.w[4][0], 0x80000000, "MSG_R1_W4");
    constant32(&r0.w[5][0], 0x00000000, "MSG_R1_W5");
    constant32(&r0.w[6][0], 0x00000000, "MSG_R1_W6");
    constant32(&r0.w[7][0], 0x00000000, "MSG_R1_W7");
    constant32(&r0.w[8][0], 0x80000000, "MSG_R1_W8");
    constant32(&r0.w[9][0], 0x00000000, "MSG_R1_W9");
    constant32(&r0.w[10][0], 0x00000000, "MSG_R1_W10");
    constant32(&r0.w[11][0], 0x00000000, "MSG_R1_W11");
    constant32(&r0.w[12][0], 0x00000000, "MSG_R1_W12");
    constant32(&r0.w[13][0], 0x00000000, "MSG_R1_W13");
    constant32(&r0.w[14][0], 0x00000000, "MSG_R1_W14");
    constant32(&r0.w[15][0], 0x00000100, "MSG_R1_W15");
    // 限制输出
    for(int i = 0; i < 8; i++){
        constant32(r0.h_out[i], target[i], "hout_constraint");
    }
}



int main(int argc, char *argv[])
{
    std::string file_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/resource/latestblock.json"; 
    // std::string cnf_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/bitcoin.cnf";

    // // open cnf file
    // std::ofstream cnf_file;
    // cnf_file.open(cnf_name);
    // if(! cnf_file.is_open()){
    //     std::cerr << "open file failed" << std::endl;
    // }

    config_cnf = 1;
    config_use_tseitin_adders = 1;

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

    if(!block_mining.createTargetFromBits()){
        std::cerr << "create from bits failed" << std::endl;
    }

    std::cout << std::endl;

    for(int i =4; i <= 16; i++){
        nr_clauses = 0;
        nr_variables = 0;
        config_use_xor_clauses = 0;
        LEFT_BITS = i;
        
        std::string cnf_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/withour_xor/sat/random";
        cnf_name += (std::to_string(i) + "bit.cnf");
        // open cnf file
        std::ofstream cnf_file;
        cnf_file.open(cnf_name);
        if(! cnf_file.is_open()){
            std::cerr << "open file failed" << std::endl;
        }

        // bitcoin比较冗长，我直接测试一个sha256的编码有没有问题

        if(!bitcoin(input_except_nonce,
                reinterpret_cast<const uint8_t*>(block_mining.target.data())))
        {
            std::cout << "the bitcoin wrong" << std::endl;
            return -1;
        }

        outPutHex("target output in uint8", block_mining.target.data(), block_mining.target.size());
        // 此处是新写的测试，只包括一轮
        // todo
        // 1. 测试一轮sha256直接计算出来的是否满足, 一个sha256是没有问题的
        // 2. 如果满足就是赋值和约束的问题了。

        // testSatUnit(input_except_nonce);

        std::cout <<std::dec << nr_variables << " " << nr_clauses << std::endl;
        
        // outPutHex("version", block_mining.version_bytes.data(), block_mining.version_bytes.size());
        // outPutHex("previous_hash", block_mining.previous_block_hash_bytes.data(), block_mining.previous_block_hash_bytes.size());
        // outPutHex("merkel", block_mining.merkel_root_bytes.data(), block_mining.merkel_root_bytes.size());
        // outPutHex("time", block_mining.time_bytes.data(), block_mining.time_bytes.size());
        // outPutHex("bits", block_mining.bits_bytes.data(), block_mining.bits_bytes.size());



        if (config_cnf) {
            cnf_file
            << format("p cnf $ $\n", nr_variables, nr_clauses)
            << cnf.str();
        }
        cnf.str("");
        cnf.clear();
        nr_clauses = 0;
        nr_variables = 0;
        
        cnf_file.close();
    }
    
    // todo: 
    // 1. 限制nonce大小，大小怎么划分
    // 2. 10个求解器进行求解
    // 3. 写引入和前人工作
    


    
    return 0;
}
