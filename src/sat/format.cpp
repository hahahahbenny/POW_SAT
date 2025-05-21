#include "sat/format.hh"

std::string comment(std::string str, bool tag){

    return format("c $\n", str);
}

void format(std::ostringstream &ss, const char *s)
{
	while (*s) {
		if (*s == '$' || *s == '#')
			throw std::runtime_error("too few arguments provided to format");

		ss << *s++;
	}
}
