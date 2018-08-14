//
// Created by kourin on 2018/08/14.
//

#ifndef TINYCHAIN_CPP_UTILITY_H
#define TINYCHAIN_CPP_UTILITY_H

#include <bitset>
#include <string>
#include <algorithm>

extern std::bitset<4> hexTo4Bits(const char c);
extern std::bitset<256> hexTo256Bits(const std::string from);

#endif //TINYCHAIN_CPP_UTILITY_H
