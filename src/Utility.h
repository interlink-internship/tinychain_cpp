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

#include "TxIn.h"
#include "Transaction.h"
#include "UnspentTxOut.h"

class Block;

/*
extern std::bitset<4> hexTo4Bits(const char c);
extern std::bitset<256> hexTo256Bits(const std::string from);
extern std::string pubkeyToAddress(const std::vector<unsigned char>& bytes);
*/
extern std::shared_ptr<UnspentTxOut> findUtxoInList(std::shared_ptr<TxIn> txin, std::vector<std::shared_ptr<Transaction>>& txns);
extern bool validateSignatureForSpend(std::shared_ptr<TxIn> txin, std::shared_ptr<UnspentTxOut> utxo, std::vector<std::shared_ptr<TxOut>> txouts);

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
//extern bool calcNonce(Block&, int&, const boost::multiprecision::uint256_t, std::mutex&, bool&);

#endif //TINYCHAIN_CPP_UTILITY_H
