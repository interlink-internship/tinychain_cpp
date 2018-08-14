//
// Created by kourin on 2018/08/14.
//

#include "Utility.h"


std::bitset<4> hexTo4Bits(const char c) {
    switch(c) {
        case '0':
            return std::bitset<4>("0000");
        case '1':
            return std::bitset<4>("0001");
        case '2':
            return std::bitset<4>("0010");
        case '3':
            return std::bitset<4>("0011");
        case '4':
            return std::bitset<4>("0100");
        case '5':
            return std::bitset<4>("0101");
        case '6':
            return std::bitset<4>("0110");
        case '7':
            return std::bitset<4>("0111");
        case '8':
            return std::bitset<4>("1000");
        case '9':
            return std::bitset<4>("1001");
        case 'a':
            return std::bitset<4>("1010");
        case 'b':
            return std::bitset<4>("1011");
        case 'c':
            return std::bitset<4>("1100");
        case 'd':
            return std::bitset<4>("1101");
        case 'e':
            return std::bitset<4>("1110");
        case 'f':
            return std::bitset<4>("1111");
        case 'A':
            return std::bitset<4>("1010");
        case 'B':
            return std::bitset<4>("1011");
        case 'C':
            return std::bitset<4>("1100");
        case 'D':
            return std::bitset<4>("1101");
        case 'E':
            return std::bitset<4>("1110");
        case 'F':
            return std::bitset<4>("1111");
        default:
            return std::bitset<4>("0000");
    }
}

// most right 64 length of hex string to 256 bits
std::bitset<256> hexTo256Bits(const std::string from) {
    std::string str = from.substr(std::max(0, (int)from.size()-1-64));
    std::bitset<256> bits;

    int len = str.length();
    for(int i=len-1; i>=0; --i) {
        int offset = (len - 1 - i) * 4;
        std::bitset<4> sub = hexTo4Bits(str[i]);
        for(int j=0; j<4; ++j) {
            bits[offset + j] = sub[j];
        }
    }
    return bits;
}

