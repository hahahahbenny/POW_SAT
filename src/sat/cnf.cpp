#include "sat/cnf.h"
#include "sat/format.hh"
#include <string>
#include <fstream>
#include <iostream>

// write comment clause to cnf output stream

void Cnf::comment(std::string str)
{
    output << format("c $\n", str);
}

void Cnf::reset(){
    nr_clauses = 0;
    nr_xor_clauses = 0;
    nr_variables = 0;
    output.str("");
    output.clear();
}

void Cnf::newVars(std::string label, int x[], unsigned int n, bool decision_var){
    for (unsigned int i = 0; i < n; ++i)
        x[i] = ++nr_variables;
}

void Cnf::constant1(int r, bool value, std::string help_text){
    output << format("$$ 0\n", value ? "" : "-", r);
    nr_clauses += 1;
}

void Cnf::constant(int r[], u_int8_t value, int size, std::string help_text){
    for (unsigned int i = 0; i < 8; ++i)
    {
        output << format("$$ 0\n", (value >> i) & 1 ? "" : "-", r[i]);
        nr_clauses += 1;
    }
}

void Cnf::constant32(int r[], uint32_t value, std::string help_text){
    comment(format("constant32 (#) -> $", value, help_text));

    for (unsigned int i = 0; i < 32; ++i)
    {
        output << format("$$ 0\n", (value >> i) & 1 ? "" : "-", r[i]);

        nr_clauses += 1;
    }
}

void Cnf::newConstant(std::string label, int r[], int size, uint32_t value){
    newVars(label, r, size);
    switch (size){
    case 1: 
        constant1(*r, value, label + "constant1");
        break;
    case 32:
        constant32(r, value, label + "constant32");
        break;
    default:
        constant(r, value, size, label+"constant"+std::to_string(size));
    }
}

// clause function


void Cnf::clause(const std::vector<int> &v)
{
    for (int x : v)
    {
        output << format("$$ ", x < 0 ? "-" : "", abs(x));
    }

    output << format("0\n");

    nr_clauses += 1;
}

void Cnf::and2(int r[], int a[], int b[], unsigned int size){
    for (unsigned int i = 0; i < size; ++i)
    {
        clause(r[i], -a[i], -b[i]);
        clause(-r[i], a[i]);
        clause(-r[i], b[i]);
    }
}

void Cnf::or2(int r[], int a[], int b[], unsigned int size){
    for (unsigned int i = 0; i < size; ++i)
    {
        clause(-r[i], a[i], b[i]);
        clause(r[i], -a[i]);
        clause(r[i], -b[i]);
    }
}

void Cnf::lt2(int r[], int a[], int b[], int equal[], unsigned int size) {
    comment("lt2");
    int temp[size];
    newVars("lt", temp, size);
    for (unsigned int i = 0; i < size; ++i){
        clause(temp[i], a[i], -b[i]);
        and2(&r[i], &temp[i], &equal[i], 1);
    }
}

void Cnf::xor_clause(const std::vector<int> &v)
{
    output << format("x ");

    for (int x : v)
        output << format("$$ ", x < 0 ? "-" : "", abs(x));

    output << format("0\n");

    nr_xor_clauses += 1;
}

template <typename... Args> void xor_clause(Args... args)
{
    std::vector<int> v;
    args_to_vector(v, args...);
    xor_clause(v);
}

void Cnf::eq(int a[], int b[], unsigned int n)
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

void Cnf::xor2(int r[], int a[], int b[], unsigned int size)
{
    comment("xor2");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < size; ++i)
            xor_clause(-r[i], a[i], b[i]);
    }
    else
    {
        for (unsigned int i = 0; i < size; ++i)
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

void Cnf::xor3(int r[], int a[], int b[], int c[], unsigned int size)
{
    comment("xor3");

    if (config_use_xor_clauses)
    {
        for (unsigned int i = 0; i < size; ++i)
            xor_clause(-r[i], a[i], b[i], c[i]);
    }
    else
    {
        for (unsigned int i = 0; i < size; ++i)
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

void Cnf::xor4(int r[32], int a[32], int b[32], int c[32], int d[32])
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

void Cnf::add2(std::string label, int r[32], int a[32], int b[32])
{
    comment("add2");

    // 链式加法，看能否改成超前进位
    int c[31];
    newVars("carry", c, 31);
    int t0[31];
    newVars("t0", t0, 31);
    int t1[31];
    newVars("t1", t1, 31);
    int t2[31];
    newVars("t2", t2, 31);
    and2(c, a, b, 1);
    xor2(r, a, b, 1);
    xor2(t0, &a[1], &b[1], 31);
    and2(t1, &a[1], &b[1], 31);
    and2(t2, t0, c, 31);
    or2(&c[1], t1, t2, 30);
    xor2(&r[1], t0, c, 31);
}

void Cnf::add4(std::string label, int r[32], int a[32], int b[32], int c[32], int d[32])
{
    comment("add4");

    int t0[32];
    newVars("t0", t0, 32);

    int t1[32];
    newVars("t1", t1, 32);

    add2(label, t0, a, b);
    add2(label, t1, c, d);
    add2(label, r, t0, t1);

}

void Cnf::add5(std::string label, int r[32], int a[32], int b[32], int c[32], int d[32], int e[32])
{
    comment("add5");
        
    int t0[32];
    newVars("t0", t0, 32);

    int t1[32];
    newVars("t1", t1, 32);

    int t2[32];
    newVars("t2", t2, 32);

    add2(label, t0, a, b);
    add2(label, t1, c, d);
    add2(label, t2, t0, t1);
    add2(label, r, t2, e);

}

// output function

bool Cnf::writeFile(std::string address){
    std::ofstream cnf_file;
    cnf_file.open(address);

    if(! cnf_file.is_open()){
        std::cerr << "open file failed" << std::endl;
        return false;
    }

    cnf_file
        << format("p cnf $ $\n", nr_variables, nr_clauses)
        << output.str();
    
    cnf_file.close();
    
    return true;
}