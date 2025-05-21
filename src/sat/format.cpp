#include "sat/format.hh"

std::string comment(std::string str, bool tag){

    return format("c $\n", str);
}