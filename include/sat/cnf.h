#ifndef CNF
#define CNF

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

class Cnf{

    Cnf(): nr_variables(0), nr_clauses(0), nr_xor_clauses(0){};
    Cnf(bool use_xor) : config_use_xor_clauses(use_xor){};
    // basic function
    // void comment(std::string str);
    void newVars(std::string label, int x[], unsigned int n, bool decision_var = true);
    void newConstant(std::string label, int r[], int size, uint32_t value);
    void constant1(int r, bool value, std::string help_text);
    void constant(int r[], u_int8_t value, int size, std::string help_text);
    void constant32(int r[], uint32_t value, std::string help_text);
    void reset();

    // basic clause function
    void and2(int r[], int a[], int b[], unsigned int size);
    void or2(int r[], int a[], int b[], unsigned int size);
    void lt2(int r[], int a[], int b[], int equal[], unsigned int size);
    void eq(int a[], int b[], unsigned int n = 32);
    void xor_clause(const std::vector<int> &v);
    void xor2(int r[], int a[], int b[], unsigned int size);
    void xor3(int r[], int a[], int b[], int c[], unsigned int size = 32);
    void xor4(int r[32], int a[32], int b[32], int c[32], int d[32]);
    void add2(std::string label, int r[32], int a[32], int b[32]);
    void add4(std::string label, int r[32], int a[32], int b[32], int c[32], int d[32]);
    void add5(std::string label, int r[32], int a[32], int b[32], int c[32], int d[32], int e[32]);

    template <typename... Args> void xor_clause(Args... args)
    {
        std::vector<int> v;
        args_to_vector(v, args...);
        xor_clause(v);
    }

    template <typename... Args> void clause(Args... args){
        std::vector<int> v;
        args_to_vector(v, args...);
        clause(v);
    }

    // output function
    virtual bool writeFile(std::string address);
    
    //control config 
    bool config_use_xor_clauses = false;
    
protected:
    void clause(const std::vector<int> &v);

    template <typename T> static void args_to_vector(std::vector<T> &v) {}

    template <typename T, typename... Args> static void args_to_vector(std::vector<T> &v, T x, Args... args)
    {
        v.push_back(x);
        return args_to_vector(v, args...);
    }

    std::ostringstream output;

    uint64_t nr_variables;
    uint64_t nr_clauses;
    uint64_t nr_xor_clauses;
};


#endif 