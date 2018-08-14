//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_BLOCK_H
#define TINYCHAIN_CPP_BLOCK_H

#include <vector>
#include <string>
#include <memory>
#include <sstream>

#include "Transaction.h"
#include "sha256.h"
#include "Params.h"

class Block {
public:
    // A version integer.
    int version;

    // A hash of the previous block's header.
    std::string prevBlockHash;

    // A hash of the Merkle tree containing all txns.
    std::string markleHash;

    // A UNIX timestamp of when this block was created.
    int timestamp;

    // The difficulty target; i.e. the hash of this block header must be under
    // (2 ** 256 >> bits) to consider work proved.
    int bits;

    // The value that's incremented in an attempt to get the block header to
    // hash to a value below `bits`.
    int nonce;

    std::vector<std::shared_ptr<Transaction>> txns;

    Block()
        : version(0), prevBlockHash(""), markleHash(""), timestamp(0), bits(0), nonce(0)
    {
        txns.reserve(RESERVE_TRANSACTION_SIZE_OF_BLOCK);
    };

    Block(const int version, const std::string prevBlockHash, const std::string markleHash, const int timestamp, const int bits, const int nonce, std::vector<std::shared_ptr<Transaction>>& txns)
        : txns(txns.begin(), txns.end()), version(version), prevBlockHash(prevBlockHash), markleHash(markleHash), timestamp(timestamp), bits(bits), nonce(nonce)
        {};

    Block(const Block&);

    std::string header() {
        std::stringstream ss;
        ss << "{";
        ss << "\"version\":" << this->version << ",";
        ss << "\"prevBlockHash\":\"" << this->prevBlockHash << "\",";
        ss << "\"markleHash\":\"" << this->markleHash << "\",";
        ss << "\"timestamp\":" << this->timestamp << ",";
        ss << "\"bits\":" << this->bits << ",";
        ss << "\"nonce\":" << this->nonce;
        ss << "}";
        return ss.str();
    }

    std::string toString() {
        std::stringstream ss;
        ss << "{";
        ss << "\"version\":" << this->version << ",";
        ss << "\"prevBlockHash\":\"" << this->prevBlockHash << "\",";
        ss << "\"markleHash\":\"" << this->markleHash << "\",";
        ss << "\"timestamp\":" << this->timestamp << ",";
        ss << "\"bits\":" << this->bits << ",";
        ss << "\"nonce\":" << this->nonce << ",";

        ss << "\"txns\":[";
        int i = 0;
        for(const auto& tx: this->txns) {
            ss << (i++ > 0 ? "," : "") << tx->toString();
        }
        ss << "]";

        ss << "}";
        return ss.str();
    }

    std::string id() {
        return sha256(sha256(this->toString()));
    }

    std::string header(const int nonce) {
        const int temp = this->nonce;
        this->nonce = nonce;
        std::string header = this->header();
        this->nonce = temp;
        return header;
    }
};


#endif //TINYCHAIN_CPP_BLOCK_H
