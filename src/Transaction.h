//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_TRANSACTION_H
#define TINYCHAIN_CPP_TRANSACTION_H

#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <sstream>

#include "TxIn.h"
#include "TxOut.h"
#include "Params.h"
#include "sha256.h"


class Transaction {
public:
    std::vector<std::shared_ptr<TxIn>> txins;
    std::vector<std::shared_ptr<TxOut>> txouts;

    // The block number or timestamp at which this transaction is unlocked.
    // < 500000000: Block number at which this transaction is unlocked.
    // >= 500000000: UNIX timestamp at which this transaction is unlocked.
    int locktime;

    Transaction() {
        this->locktime = 0;
        this->txins.reserve(RESERVE_IO_SIZE_OF_TRANSACTION);
        this->txouts.reserve(RESERVE_IO_SIZE_OF_TRANSACTION);
    }

    Transaction(std::shared_ptr<TxIn>&& txin, std::shared_ptr<TxOut>&& txout) {
        this->locktime = 0;
        this->txins.reserve(RESERVE_IO_SIZE_OF_TRANSACTION);
        this->txouts.reserve(RESERVE_IO_SIZE_OF_TRANSACTION);

        this->txins.push_back(txin);
        this->txouts.push_back(txout);
    }

    ~Transaction() {
        txins.clear();
        txouts.clear();
    }

    std::string toString() {
        std::stringstream ss;
        ss << "{";
        ss << "\"txins\":[";
        for(const auto& txin: this->txins) {
            ss << txin->toString();
        }
        ss << "],";

        ss << "\"txouts\":[";
        for(const auto& txout: this->txouts) {
            ss << txout->toString();
        }
        ss << "],";
        ss << "\"locktime\":" << this->locktime;
        ss << "}";
        return ss.str();
    }

    static std::shared_ptr<Transaction> createCoinbase(const std::string payToAddr, const int value, const int height) {
        auto transaction = std::make_shared<Transaction>();
        auto txin = std::make_shared<TxIn>(std::make_shared<OutPoint>("", 0), std::to_string(height), "", 0);
        auto txout = std::make_shared<TxOut>(value, payToAddr);
        transaction->txins.push_back(txin);
        transaction->txouts.push_back(txout);

        return transaction;
    }

    bool isCoinbase() {
        return this->txins.size() == 1 && this->txins[0]->toSpend->txid.empty();
    }

    std::string id() {
        return sha256(sha256(this->toString()));
    }

    void validBasics(const bool asCoinbase) {
        if(txouts.size() == 0 && (!asCoinbase && txins.size() == 0)) {
            throw std::runtime_error("Missing txouts or txins");
        }
        if(this->toString().length() > MAX_BLOCK_SERIALIZED_SIZE) {
            throw std::runtime_error("Too large");
        }

        int sum = 0;
        for(const auto& txout: this->txouts) {
            sum += txout->value;
        }
        if(sum > MAX_MONEY) {
            throw std::runtime_error("Spend value too high");
        }
    }
};


#endif //TINYCHAIN_CPP_TRANSACTION_H
