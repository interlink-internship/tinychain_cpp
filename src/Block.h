//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_BLOCK_H
#define TINYCHAIN_CPP_BLOCK_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>

#include "Params.h"
#include "Transaction.h"
#include "Utility.h"

class Block {
public:
    // A version integer.
    int version;

    // A hash of the previous block's header.
    std::string prevBlockHash;

    // A hash of the Merkle tree containing all txns.
    std::string markleHash;

    // A UNIX timestamp of when this block was created.
    std::time_t timestamp;

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

    Block(const int version, const std::string prevBlockHash, const std::string markleHash, const std::time_t timestamp, const int bits, const int nonce, std::vector<std::shared_ptr<Transaction>>& txns)
        : txns(txns.begin(), txns.end()), version(version), prevBlockHash(prevBlockHash), markleHash(markleHash), timestamp(timestamp), bits(bits), nonce(nonce) {;};

    Block(const Block&);

    // exception using validation
    class BlockValidationException : public std::runtime_error {
    public:
        std::shared_ptr<Block> toOrphan;
        BlockValidationException(const char *_Message)
                : toOrphan(nullptr), runtime_error(_Message) {};

        BlockValidationException(const char *_Message, std::shared_ptr<Block> block)
                : toOrphan(block), runtime_error(_Message) {};
    };

    int validate(const std::time_t medianTimestamp, const bool hasActiveChain, const int activeChainIdx) {
        if(this->txns.size() == 0) {
            throw new BlockValidationException("txns empty");
        }
        if((this->timestamp - time(NULL)) > MAX_FUTURE_BLOCK_TIME) {
            throw new BlockValidationException("Block timestamp too far in future");
        }

        std::vector<unsigned char> idBytes;
        hexStringToBytes(this->id(), idBytes);
        boost::multiprecision::uint256_t idValue = 0;
        for(const auto byte: idBytes) {
            idValue *= 256;
            idValue += byte;
        }
        auto right = boost::multiprecision::uint256_t("1");
        right <<= (256 - this->bits);
        if(idValue > right) {
            throw new BlockValidationException("Block header doesn't satisfy bits");
        }

        /*
        //check id(hex) is bigger than 2^(256-this->bits)
        std::bitset<256> idBits = hexTo256Bits(this->id());
        bool isBigger = false;
        for(int i=256-this->bits; i<256; ++i) {
            if(idBits[i]) {
                isBigger = true;
                break;
            }
        }
        if(!isBigger && idBits[256 - this->bits]) {
            for(int i=0; i<256-this->bits; ++i) {
                if(idBits[i]) {
                    isBigger = true;
                    break;
                }
            }
        }
        if(isBigger) {
            throw new BlockValidationException("Block header doesn't satisfy bits");
        }
        */

        const int numTxns = this->txns.size();
        //first transaction must be coinbase, others must not be coinbase
        if(numTxns == 0) {
            throw new BlockValidationException("First txn must be coinbase and no more");
        } else{
            if(!this->txns[0]->isCoinbase()) {
                throw new BlockValidationException("First txn must be coinbase and no more");
            }
            for(int i=1; i<numTxns; ++i) {
                if(this->txns[i]->isCoinbase()) {
                    throw new BlockValidationException("First txn must be coinbase and no more");
                }
                try {
//                    this->txns[i]->validate(this->txns, false);
                } catch (std::runtime_error error) {

                    const std::string message = "Transaction(" + this->txns[i]->id() + ") failed to validate";
                    std::cout << message << "\n";
                    throw new BlockValidationException(message.c_str());
                }
            }
        }

        std::string txnId;
        try {
            for(int i=0; i<numTxns; ++i) {
                txnId = this->txns[i]->id();
                this->txns[i]->validateBasics(i == 0);
            }
        } catch (std::runtime_error error) {
            std::cout << "Transaction(" << txnId << ") in Block(" << this->header() << ") failed to validate\n";
            throw new BlockValidationException(("Invalid txn" + txnId).c_str());
        }

        if(this->getMarkleHash() != this->markleHash) {
            throw new BlockValidationException("Merkle hash invalid");
        }


        if(this->timestamp <= medianTimestamp) {
            throw new BlockValidationException("timestamp too old");
        }

        return 0;
    }

    std::string header() {
        return this->header(this->nonce);
    }

    std::string header(int nonce) {
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
        return sha256DoubleHash(this->header());
    }

    void setMarkleHash() {
        this->markleHash = this->getMarkleHash();
    }

    std::string getMarkleHash() {
        return getMarkleHash(0, this->txns.size() - 1);
    }

    std::string getMarkleHash(int left, int right) {
        std::string leftHash, rightHash;
        if(right-left <= 1) {
            leftHash = this->txns[left]->id();
            rightHash = this->txns[right]->id();
        } else {
            int mid = (right + left)/2;
            leftHash = this->getMarkleHash(left, mid);
            rightHash = this->getMarkleHash(mid + 1, right);
        }
        return sha256DoubleHash(leftHash + rightHash);
    }
};


#endif //TINYCHAIN_CPP_BLOCK_H
