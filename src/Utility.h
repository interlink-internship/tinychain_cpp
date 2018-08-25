//
// Created by kourin on 2018/08/14.
//

#ifndef TINYCHAIN_CPP_UTILITY_H
#define TINYCHAIN_CPP_UTILITY_H

#include <bitset>
#include <string>
#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <mutex>
#include <random>
#include <secp256k1.h>

#include "TxIn.h"
#include "TxOut.h"
#include "UnspentTxOut.h"
#include "Transaction.h"

class Block;
class Transaction;

/*
extern std::bitset<4> hexTo4Bits(const char c);
extern std::bitset<256> hexTo256Bits(const std::string from);
extern std::string pubkeyToAddress(const std::vector<unsigned char>& bytes);
*/

extern bool validateSignatureForSpend(std::shared_ptr<TxIn>, std::shared_ptr<UnspentTxOut>, std::vector<std::shared_ptr<TxOut>>&);

extern char hexCharToByte(const char);

extern void hexStringToBytes(std::string, std::vector<unsigned char>&);

extern char byteToHexChar(unsigned char);

extern void bytesToHexString(std::vector<unsigned char>&, std::vector<char>&);

extern void bytesSha256(unsigned char*, int, unsigned char*);

extern void bytesRipemd160(unsigned char*, int, unsigned char*);

extern std::string sha256DoubleHash(std::string);

extern std::string base58Encode(unsigned char*, int);

extern std::string base58EncodeCheck(unsigned char*, int);

extern std::string pubkeyToAddress(std::vector<unsigned char>&);

template<typename T>
void integerToBytes(T value, size_t size, std::vector<unsigned char>& bytes) {
    bytes.resize(size);
    unsigned char mask = -1;
    for(size_t i = 0; i<size; ++i) {
        std::cout << value << "->" << (mask & value) << std::endl;
        bytes[i] = (mask & value);
        value = value >> 8;
    }
    std::reverse(bytes.begin(), bytes.end());
}

template<typename T>
T bytesToInteger(size_t size, const unsigned char* bytes) {
    T value = 0;
    for(int i=0; i<size; ++i) {
        value = value << 8;
        value = value + T(bytes[i]);
    }
    return value;
}

extern void generateRandomBits(const size_t, std::vector<unsigned char>&);

extern void convertPrivateKeyToPublicKey(std::vector<unsigned char>&, std::vector<unsigned char>&);

extern void generateSignature(std::vector<unsigned char>&, std::vector<unsigned char>&, std::vector<unsigned char>&);

extern bool verifySignature(std::vector<unsigned char>& signature, std::vector<unsigned char>&, std::vector<unsigned char>);



#endif //TINYCHAIN_CPP_UTILITY_H
