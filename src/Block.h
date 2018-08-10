//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_BLOCK_H
#define TINYCHAIN_CPP_BLOCK_H

#include <string>
#include "Transaction.h"
#include "sha256.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

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

    std::vector<Transaction> txns;

    template<class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(version), CEREAL_NVP(prevBlockHash), CEREAL_NVP(markleHash), CEREAL_NVP(timestamp), CEREAL_NVP(bits), CEREAL_NVP(nonce), CEREAL_NVP(txns));
    }

    std::string serialize() {
        std::stringstream ss;
        {
            cereal::JSONOutputArchive o_archive(ss);
            o_archive(*this);
        }
        return ss.str();
    }

    std::string id() {
        return sha256(sha256(this->serialize()));
    }

    std::string header() {
        return this->serialize();
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
