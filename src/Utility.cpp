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

void generateRandomBits(const size_t bitLen, std::vector<unsigned char>& bits) {
    size_t bytes = (bitLen/8) + (bitLen % 8 != 0);
    bits.resize(bytes);

    std::random_device rnd;
    std::mt19937_64 mt(rnd());

    int chunckNum = bytes/8 + (bytes % 8 != 0);
    std::uint64_t mask = 0b11111111;
    for(int i=0; i<chunckNum; ++i) {
        std::uint64_t value = mt();
        for(int k=0; k < 8; ++k) {
            if(i*8+k >= bytes) break;
            bits[i*8 + k] = (value >> 8*(8-k-1)) & mask;
        }
    }
}

void convertPrivateKeyToPublicKey(std::vector<unsigned char>& privateKeyBytes, std::vector<unsigned char>& publicKeyBytes) {
    publicKeyBytes.resize(64);
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_pubkey pubkey;
    secp256k1_ec_pubkey_create(ctx, &pubkey, privateKeyBytes.data());
    std::copy(pubkey.data, pubkey.data + 64, publicKeyBytes.begin());
}

void generateSignature(std::vector<unsigned char>& privateKey, std::vector<unsigned char>& message, std::vector<unsigned char>& signature) {
    signature.resize(64);

    auto privateKeyBytes = privateKey.data();
    auto messageBytes = message.data();
    secp256k1_ecdsa_signature sig;
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_ecdsa_sign(ctx, &sig, messageBytes, privateKeyBytes, NULL, NULL);
    std::copy(sig.data, sig.data + 64, signature.begin());
}

bool verifySignature(std::vector<unsigned char>& signature, std::vector<unsigned char>& message, std::vector<unsigned char> publicKey) {
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
    secp256k1_ecdsa_signature sig;
    std::copy(signature.begin(), signature.end(), sig.data);
    auto msg = message.data();
    secp256k1_pubkey pub;
    std::copy(publicKey.begin(), publicKey.end(), pub.data);

    return secp256k1_ecdsa_verify(ctx, &sig, msg, &pub) == 1;
}


bool validateSignatureForSpend(std::shared_ptr<TxIn> txin, std::shared_ptr<UnspentTxOut> utxo, std::vector<std::shared_ptr<TxOut>>& txouts) {
    auto pubkeyAsAddr = pubkeyToAddress(txin->unlockPk);
    if(pubkeyAsAddr != utxo->toAddr) {
        throw new std::runtime_error("Pubkey doesn't match");
    }

    std::vector<char> hexPk;
    bytesToHexString(txin->unlockPk, hexPk);

    std::stringstream ss;
    ss << txin->toSpend->toString();
    ss << txin->sequence;
    ss << std::string(hexPk.begin(), hexPk.end());
    ss << "[";
    int cnt = 0;
    for(const auto& txout: txouts) {
        if(cnt++ != 0) {
            ss << ",";
        }
        ss << txout->toString();
    }
    ss << "]";
    std::string message = sha256DoubleHash(ss.str());
    std::vector<unsigned char> msg;
    hexStringToBytes(message, msg);

    if(!verifySignature(txin->unlockSig, msg, txin->unlockPk)) {
        throw new std::runtime_error("Signature doesn't match");
    }
    return true;
}