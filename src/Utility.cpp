//
// Created by kourin on 2018/08/14.
//

#include "Utility.h"

/*
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
 */

std::shared_ptr<UnspentTxOut> findUtxoInList(std::shared_ptr<TxIn> txin, std::vector<std::shared_ptr<Transaction>>& txns) {
    auto txid = txin->toSpend->txid;
    auto txoutIdx = txin->toSpend->txoutIdx;

    std::shared_ptr<TxOut> txout = nullptr;
    for(const auto& tx: txns) {
        if(tx->id() == txid) {
            txout = tx->txouts[txoutIdx];
            break;
        }
    }
    return (txout != nullptr
            ? std::make_shared<UnspentTxOut>(txout->value, txout->toAddress, txid, txoutIdx, false, -1)
            : nullptr);
}

bool validateSignatureForSpend(std::shared_ptr<TxIn> txin, std::shared_ptr<UnspentTxOut> utxo, std::vector<std::shared_ptr<TxOut>> txouts) {
    return false;
}

char hexCharToByte(const char c) {
    if(c >= '0' && c <= '9') {
        return c - '0';
    } else if(c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if(c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        return 0;
    }
}

void hexStringToBytes(std::string from, std::vector<unsigned char>& vec) {
    int len = from.size();
    vec.resize(len/2 + len%2, 0);
    for(int i=0; i<len; ++i) {
        unsigned char byte = hexCharToByte(from[i]);
        unsigned char mask = (i % 2 == 0) ? 0b11110000 : 0b00001111;
        if(i % 2 == 0) {
            vec[i/2] |= (mask & (byte << 4));
        } else {
            vec[i/2] |= (mask & byte);
        }
    }
}

char byteToHexChar(unsigned char byte) {
    byte = byte & 0b00001111;
    if(byte >= 0 && byte <= 9) {
        return '0' + byte;
    } else if(byte >= 10 && byte <= 15) {
        return 'a' + (byte - 10);
    } else {
        return 'x';
    }
}

void bytesToHexString(std::vector<unsigned char>& from, std::vector<char>& to) {
    int len = from.size();
    to.resize(len*2);
    for(int i=0; i<len; ++i) {
        unsigned char byte = from[i];
        char upper = byteToHexChar(byte >> 4);
        char lower = byteToHexChar(byte);
        to[i*2]     = upper ;
        to[i*2 + 1] = lower;
    }
}

void bytesSha256(unsigned char* fromBytes, int len, unsigned char* toBytes) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, fromBytes, len);
    SHA256_Final(toBytes, &sha256);
}

void bytesRipemd160(unsigned char* fromBytes, int len, unsigned char* toBytes) {
    RIPEMD160_CTX ripemd160;
    RIPEMD160_Init(&ripemd160);
    RIPEMD160_Update(&ripemd160, fromBytes, len);
    RIPEMD160_Final(toBytes, &ripemd160);
}

std::string sha256DoubleHash(std::string from) {
    std::vector<unsigned char> fromBytes;
    hexStringToBytes(from, fromBytes);

    unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
    bytesSha256(fromBytes.data(), fromBytes.size(), sha256Hash);
    bytesSha256(sha256Hash, SHA256_DIGEST_LENGTH, sha256Hash);

    std::vector<unsigned char> sha256BytesVec(sha256Hash, sha256Hash + SHA256_DIGEST_LENGTH);
    std::vector<char> sha256HexVec;
    bytesToHexString(sha256BytesVec, sha256HexVec);
    return std::string(sha256HexVec.begin(), sha256HexVec.end());
}

std::string base58Encode(unsigned char* bytes, int len) {
    const std::string codes = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::vector<char> data;
    data.reserve(len);

    boost::multiprecision::uint256_t value = 0;
    for(int i=0; i<len; ++i) {
        value *= 256;
        value += bytes[i];
    }

    while(value > 0) {
        auto x = value / 58;
        auto r = value % 58;
        auto idx = std::stoi(r.str());
        data.push_back(codes[idx]);
        value = x;
    }

    int i = 0;
    while(bytes[i] == 0) {
        data.push_back('1');
        ++i;
    }
    return std::string(data.rbegin(), data.rend());
}

std::string base58EncodeCheck(unsigned char* bytes, int len) {
    unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
    bytesSha256(bytes, len, sha256Hash);
    bytesSha256(sha256Hash, SHA256_DIGEST_LENGTH, sha256Hash);

    unsigned char dijest[len + 4];
    for(int i=0; i<len; ++i) {
        dijest[i] = bytes[i];
    }
    for(int i=0; i<4; ++i) {
        dijest[len + i] = sha256Hash[i];
    }
    return base58Encode(dijest, len + 4);
}

std::string pubkeyToAddress(std::vector<unsigned char>& bytes) {
    //sha256
    auto bytesArray = bytes.data();
    unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
    bytesSha256(bytesArray, bytes.size(), sha256Hash);

    //ripemd 160
    unsigned char ripemdHash[RIPEMD160_DIGEST_LENGTH];
    bytesRipemd160(sha256Hash, SHA256_DIGEST_LENGTH, ripemdHash);

    unsigned char data[RIPEMD160_DIGEST_LENGTH + 1];
    data[0] = 0;
    for(int i=1; i<=RIPEMD160_DIGEST_LENGTH; ++i) {
        data[i] = ripemdHash[i-1];
    }

    return base58EncodeCheck(data, RIPEMD160_DIGEST_LENGTH + 1);
}

/*
bool calcNonce(Block& block, int& nonce, const boost::multiprecision::uint256_t target, std::mutex& mineInterrupt, bool& isMineInterrupt) {
    while(true) {
        auto header = block.header(nonce);
        std::vector<unsigned char> bytes;
        hexStringToBytes(header, bytes);
        unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
        bytesSha256(bytes.data(), bytes.size(), sha256Hash);
        bytesSha256(sha256Hash, SHA256_DIGEST_LENGTH, sha256Hash);

        boost::multiprecision::uint256_t value = 0;
        for(int i=0; i<SHA256_DIGEST_LENGTH; ++i) {
            value *= 256;
            value += sha256Hash[i];
        }
        if(value < target) {
            break;
        }

        ++nonce;
        if(nonce % 100000 == 0) {
            mineInterrupt.lock();
            bool isMineInterrupt = isMineInterrupt;
            mineInterrupt.unlock();

            if(isMineInterrupt) {
                std::cout << "[mining] interrupted" << std::endl;
                mineInterrupt.lock();
                bool isMineInterrupt = false;
                mineInterrupt.unlock();
                return false;
            }
        }
    }
    return true;
}
*/



