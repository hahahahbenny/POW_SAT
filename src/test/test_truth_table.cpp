#include <cstdint>
#include <iomanip>
#include <iostream>

using namespace std;

int main(){
    std::cout <<setw(4)<< "x" << setw(4)<< " y" << setw(4)<< " z" << setw(4)<< " ch" << setw(4)<< "res"<< std::endl;
    for(uint8_t i = 0; i < 8; i++){
        uint8_t x = (i & 4) >> 2;
        uint8_t y = (i & 2) >> 1;
        uint8_t z = (i & 1);
        uint8_t ch = ((x & y) ^ (~x & z)) & 1;
        cout<<setw(4)<< static_cast<int>(x) << setw(4)<< static_cast<int>(y) <<setw(4)<< static_cast<int>(z) << setw(4)<<  static_cast<int>(ch)<< setw(4) << static_cast<int>(ch) << endl;
    }

    return 0;
}