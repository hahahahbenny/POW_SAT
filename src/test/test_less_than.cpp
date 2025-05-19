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
static int nr_variables_pre = 0;
static unsigned int nr_clauses = 0;
static unsigned int nr_xor_clauses = 0;
static unsigned int nr_constraints = 0;
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
static void comment(std::string str)
{
    cnf << format("c $\n", str);
    opb << format("* $\n", str);
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

static void constant8(int r[], u_int8_t value){
    for (unsigned int i = 0; i < 8; ++i)
    {
        cnf << format("$$ 0\n", (value >> i) & 1 ? "" : "-", r[i]);

        nr_clauses += 1;
        nr_constraints += 1;
    }
}


static void nxor2(int r[], int a[], int b[], unsigned int n){
    comment("nxor2");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < n; ++i)
            xor_clause(r[i], a[i], b[i]);
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


static void lt2(int r[], int a[], int b[], int equal[], unsigned int n) {
    comment("lt2");
    int temp[n];
    new_vars("lt", temp, n);
    for (unsigned int i = 0; i < n; ++i){
        clause(temp[i], a[i], -b[i]);
        and2(&r[i], &temp[i], &equal[i], 1);
    }
}


static void new_constant(std::string label, int r[32], uint32_t value)
{
    new_vars(label, r, 32);
    constant32(r, value, label);
}

int main(){
    config_cnf = 1;
    nr_clauses = 0;
    nr_variables = 0;
    config_use_xor_clauses = 1;
    std::string cnf_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/bitcoin_testLessThan.cnf";
    u_int8_t target[32] = {0, 0, 0, 0, 0, 0, 0, 0, 
                            0, 2, 0x5c, 0xed, 0, 0, 0, 0, 
                            0, 0, 0, 0, 0, 0, 0, 0, 
                            0, 0 ,0 ,0 ,0, 0, 0, 0};

    int h_out[8][32];

    new_constant("hout", &h_out[0][0], 0xc7437a17);
    new_constant("hout", &h_out[1][0], 0x43fbcfe3);
    new_constant("hout", &h_out[2][0], 0x9f295d24);
    new_constant("hout", &h_out[3][0], 0x54eb7063);
    new_constant("hout", &h_out[4][0], 0xcc96d219);
    new_constant("hout", &h_out[5][0], 0x944d0100);
    new_constant("hout", &h_out[6][0], 0x00000000);
    new_constant("hout", &h_out[7][0], 0x00000000);

    std::ofstream cnf_file;
    cnf_file.open(cnf_name);
    if(! cnf_file.is_open()){
        std::cerr << "open file failed" << std::endl;
    }

    // for(int i = 0; i < 8; i++){
    //     new_vars("hout", h_out[i], 32);
    // }

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
        std::cout << "the pos == " << i << "\n"; 
        // std::cout  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(temp) << " ";
        if(temp == 0){
            constant8((&(h_out[7-(i/4)][8*(i%4)])), 0x00);
            continue;
        }
        
        // 数多少个位
        int last_pos = 31;
        for(; last_pos >= i && !target[last_pos]; last_pos--);
        std::cout << "the real interval == " << last_pos-i << std::endl;

        while(i <= last_pos){
            temp = target[i];
            
            for(int k = 0; k < 8; k++){
                bool left = temp & 0x80;
                // std::cout << "pos is" << std::endl;
                std::cout<< std::dec << 7-(i/4) << " " << (8*(i%4)+k) <<" " << left << std::endl;
                constant(target_bit[pos_temp], left);
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
    cnf_file.close();
    return 0;
}