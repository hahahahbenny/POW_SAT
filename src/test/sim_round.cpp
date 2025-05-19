#include <cassert>
#include <cmath>
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

static std::ostringstream cnf;
static std::ostringstream opb;

static int nr_variables = 0;
static unsigned int nr_clauses = 0;
static unsigned int nr_xor_clauses = 0;
static unsigned int nr_constraints = 0;

template <typename T> static void args_to_vector(std::vector<T> &v) {}

template <typename T, typename... Args> static void args_to_vector(std::vector<T> &v, T x, Args... args)
{
    v.push_back(x);
    return args_to_vector(v, args...);
}


static void comment(std::string str)
{
    cnf << format("c $\n", str);
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


static void new_vars(std::string label, int x[], unsigned int n, bool decision_var = true)
{
    for (unsigned int i = 0; i < n; ++i)
        x[i] = ++nr_variables;

    comment(format("var $/$ $", x[0], n, label));
}


static void constant(int r, bool value)
{
    cnf << format("$$ 0\n", value ? "" : "-", r);

    nr_clauses += 1;
    nr_constraints += 1;
}


static void constant4(int r[], u_int8_t value, std::string helptext){

    comment(format("constant4 (#) -> $", value, helptext));
    for (unsigned int i = 0; i < 4; ++i)
    {
        cnf << format("$$ 0\n", (value >> i) & 1 ? "" : "-", r[i]);

        nr_clauses += 1;
        nr_constraints += 1;
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

static void and2(int r[], int a[], int b[], unsigned int n)
{
    comment("and2");
    for (unsigned int i = 0; i < n; ++i)
    {
        clause(r[i], -a[i], -b[i]);
        clause(-r[i], a[i]);
        clause(-r[i], b[i]);
    }
}


static void xor2(int r[], int a[], int b[], unsigned int n)
{
    comment("xor2");

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


static void xor3(int r[], int a[], int b[], int c[], unsigned int n = 32)
{
    comment("xor3");

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


static void eq(int a[], int b[], unsigned int n = 32)
{
    comment("eq");
    for (unsigned int i = 0; i < n; ++i)
    {
        clause(-a[i], b[i]);
        clause(a[i], -b[i]);
    }
}

static void new_constant4(std::string label, int r[4], uint32_t value)
{
    new_vars(label, r, 4);
    constant4(r, value, label);
}


static void add2(std::string label, int r[4], int a[4], int b[4])
{
    comment("add2");
    int c[3];
    new_vars("add_carry", c, 3);
    int t0[3];
    new_vars("add_temp0", t0, 3);
    int t1[3];
    new_vars("add_temp1", t1, 3);
    int t2[3];
    new_vars("add_temp2", t2, 3);
    and2(c, a, b, 1);
    xor2(r, a, b, 1);
    xor2(t0, &a[1], &b[1], 3);
    and2(t1, &a[1], &b[1], 3);
    and2(t2, t0, c, 3);
    or2(&c[1], t1, t2, 2);
    xor2(&r[1], t0, c, 3);
}

static void add3(std::string label, int r[4], int a[4], int b[4], int c[4])
{
    comment("add3");
    int t0[4];
    new_vars("add3_t0", t0, 4);
    
    add2(label, t0, a, b);
    add2(label, r, t0, c);
}

static void fct_SHA256_SHR(int r[4], unsigned int bits, int word[4])
{
    // return word >> bits;
    for (unsigned int i = 0; i < 4 - bits; i++)
        r[i] = word[i + bits];
    for (unsigned int i = 4 - bits; i < 4; ++i)
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

static void fct_SHA256_ROTR(int r[4], unsigned int bits, int word[4])
{
    // return (((word) >> (bits)) | ((word) << (32-(bits))));
    for (unsigned int i = 0; i < 4; ++i)
        r[i] = word[(i + bits) % 4];
}

/* Define the SHA SIGMA and sigma macros as function */
static void fct_SHA256_SIGMA0(int r[4], int word[4])
{
    // return fct_SHA256_ROTR( 2,word) ^ fct_SHA256_ROTR(13,word) ^
    // fct_SHA256_ROTR(22,word);

    int tmp1[4];
    // new_vars(format("tmp1 fct_SHA256_SIGMA0"), tmp1, 32);
    int tmp2[4];
    // new_vars(format("tmp2 fct_SHA256_SIGMA0"), tmp2, 32);

    fct_SHA256_ROTR(tmp1, 2, word);
    fct_SHA256_ROTR(tmp2, 3, word);

    xor2(r, tmp1, tmp2, 4);
}

static void fct_SHA_Ch(int r[4], int x[4], int y[4], int z[4])
{
    // return (x & (y ^ z)) ^ z;
    comment("before ch function\n");

    int tmp1[4];
    new_vars(format("tmp1 fct_SHA_Ch"), tmp1, 4);
    int tmp2[4];
    new_vars(format("tmp2 fct_SHA_Ch"), tmp2, 4);

    xor2(tmp1, y, z, 4);
    and2(tmp2, x, tmp1, 4);
    xor2(r, tmp2, z, 4);

    comment("after ch function\n");
}

// 编码简化版轮函数
int main(){

    // 初始化基本变量
    int A[2][4];
    int B[2][4];
    int C[2][4];
    int W[4];
    int k[4];
    int Ch[4];
    int SIGMA0[4];
    int T1[4];
    int T2[4];
        
    // new var
    new_vars(format("A0"), A[0], 4);
    new_vars(format("B0"), B[0], 4);
    new_vars(format("C0"), C[0], 4);
    new_vars(format("k0"), k, 4);
    new_vars(format("w0"), W, 4);
    new_vars(format("SIGMA0"), SIGMA0, 4);
    new_vars(format("T1"), T1, 4);
    new_vars(format("T2"), T2, 4);
    new_vars(format("Ch"), Ch, 4);

    new_vars(format("A1"), A[1], 4);
    new_vars(format("B1"), B[1], 4);
    new_vars(format("C1"), C[1], 4);
    
    // constants
    constant4(A[0], 0x01, "init A");
    constant4(B[0], 0x02, "init B");
    constant4(C[0], 0x03, "init C");
    constant4(W, 0x04, "init w");
    constant4(k, 0x05, "init K");
    
    
    comment("after init constant");

    // round compute
    fct_SHA256_SIGMA0(SIGMA0, A[0]);
    fct_SHA_Ch(Ch, A[0], B[0], C[0]);

    add3("compute T1", T1, C[0], k, W);
    // temp2 = SHA256_SIGMA0 (A) + SHA_Maj (A, B, C);

    add2("compute T2", T2, SIGMA0, Ch);

    comment("after compute T1 and T2");
    // C = B;
    eq(C[1], B[0], 4);
    // B = A;
    eq(B[1], A[0], 4);
    // A = temp1 + temp2;
    add2("compute A1", A[1], T1, T2);

    comment("after get A1toC1");

    std::string cnf_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/simple_round.cnf";
    // open cnf file
    std::ofstream cnf_file;
    cnf_file.open(cnf_name);
    if(! cnf_file.is_open()){
        std::cerr << "open file failed" << std::endl;
    }
    
    std::cout << nr_variables << " " << nr_clauses << std::endl;
    
    cnf_file
    << format("p cnf $ $\n", nr_variables, nr_clauses)
    << cnf.str();

    cnf_file.close();
    
}
