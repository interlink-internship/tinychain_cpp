//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_TRANSACTION_H
#define TINYCHAIN_CPP_TRANSACTION_H


#include "TxIn.h"
#include "TxOut.h"
#include "Params.h"
#include "sha256.h"

#include <vector>
#include <string>
#include <stdexcept>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>


class Transaction {
public:
    std::vector<TxIn> txins;
    std::vector<TxOut> txouts;

    // The block number or timestamp at which this transaction is unlocked.
    // < 500000000: Block number at which this transaction is unlocked.
    // >= 500000000: UNIX timestamp at which this transaction is unlocked.
    int locktime;

    bool isCoinbase() {
        return this->txins.size() == 1
               && this->txins[0].toSpend.txid.empty();
    }

    Transaction createCoinbase(const std::string payToAddr, const int value, const int height) {
        auto newTransaction = new Transaction();

        auto txin = TxIn(OutPoint("", 0), std::to_string(height), "", 0);
        auto txout = TxOut(value, payToAddr);

        auto transaction = Transaction();
        transaction.txins.emplace_back(txin);
        transaction.txouts.emplace_back(txout);

        return transaction;
    }

    Transaction() {
        this->locktime = 0;
        this->txins = std::vector<TxIn>(0);
        this->txouts = std::vector<TxOut>(0);
    }

    template<class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(txins), CEREAL_NVP(txouts), CEREAL_NVP(locktime));
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

    void validBasics(const bool asCoinbase) {
        if(txouts.size() == 0 && (!asCoinbase && txins.size() == 0)) {
            throw std::runtime_error("Missing txouts or txins");
        }
        if(this->serialize().length() > MAX_BLOCK_SERIALIZED_SIZE) {
            throw std::runtime_error("Too large");
        }

        int sum = 0;
        for(auto txout: this->txouts) {
            sum += txout.value;
        }
        if(sum > MAX_MONEY) {
            throw std::runtime_error("Spend value too high");
        }
    }

};


#endif //TINYCHAIN_CPP_TRANSACTION_H
