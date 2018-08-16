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
#include <exception>

#include "Params.h"
#include "OutPoint.h"
#include "TxIn.h"
#include "TxOut.h"

//#include "UtxoSet.h"
//#include "Utility.h"
//#include "Mempool.h"

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

    //it is fake
    std::string id() {
        return "";
    }
    /*
    std::string id() {
        return sha256DoubleHash(this->id())
    }
    */

    void validateBasics(const bool asCoinbase) {
        if(txouts.size() == 0 && (!asCoinbase && txins.size() == 0)) {
            throw std::runtime_error("Missing txouts or txins");
        }
        if(this->toString().length() > MAX_BLOCK_SERIALIZED_SIZE) {
            throw std::runtime_error("Too large");
        }

        long sum = 0;
        for(const auto& txout: this->txouts) {
            sum += txout->value;
        }
        if(sum > MAX_MONEY) {
            throw std::runtime_error("Spend value too high");
        }
    }
/*
    // exception using validation
    class TransactionValidationException : public std::runtime_error {
        public:
            bool isOrphen;
            TransactionValidationException(const char *_Message)
                    : isOrphen(false), runtime_error(_Message) {};
            TransactionValidationException(const char *_Message, const bool isOrphen)
                    : isOrphen(isOrphen), runtime_error(_Message) {};
    };

    void validate(UtxOSet& utxoSet, Mempool& mempool, const int currentHeight) {
        return this->validate(utxoSet, mempool, currentHeight, std::vector<std::shared_ptr<Transaction>>(), false, true);
    }

    void validate(UtxoSet& utxoSet, Mempool& mempool, const int currentHeight, std::vector<std::shared_ptr<Transaction>>& slidingsInBlock) {
        return this->validate(utxoSet, mempool, currentHeight, slidingsInBlock, false, true);
    }

    void validate(UtxoSet& utxoSet, Mempool& mempool, const int currentHeight, std::vector<std::shared_ptr<Transaction>>& slidingsInBlock, const bool allowUtxoFromMempool) {
        return this->validate(utxoSet, mempool, currentHeight, slidingsInBlock, false, allowUtxoFromMempool);
    }

    void validate(UtxoSet& utxoSet, Mempool& mempool, const int currentHeight, std::vector<std::shared_ptr<Transaction>>& slidingsInBlock, const bool asCoinbase, const bool allowUtxoFromMempool) {
        // Validate a single transaction. Used in various contexts, so the
        // parameters facilitate different uses.
        this->validateBasics(asCoinbase);

        long availableToSpend = 0;
        long i = 0;
        for(const auto& txin: this->txins) {
            auto utxo = utxoSet.get(txin->toSpend->txid, txin->toSpend->txoutIdx);
            if(utxo == nullptr && slidingsInBlock.size() > 0) {
                utxo = findUtxoInList(txin, slidingsInBlock);
            }
            if(utxo == nullptr && allowUtxoFromMempool) {
                utxo = mempool.findUtxo(txin);
            }

            if(utxo == nullptr) {
                const auto message = "Could find no UTXO for TxIn[" + std::to_string(i) + "] -- orphaning txn";
                throw new TransactionValidationException(message.c_str(), true);
            }

            if(utxo->isCoinbase && (currentHeight - utxo->height) < COINBASE_MATURITY) {
                throw new TransactionValidationException("Coinbase UTXO not ready for spend");
            }

            try {
                validateSignatureForSpend(txin, utxo, this->txouts);
            } catch(TxUnlockException e) {
                const auto message = txin->toString() + " is not a valid spend of " + utxo->toString();
                throw new TransactionValidationException(message.c_str());
            }

            availableToSpend += utxo->value;
            ++i;
        }

        long sum = 0;
        for(const auto& txout: this->txouts) {
            sum += txout->value;
        }
        if(availableToSpend < sum) {
            throw new TransactionValidationException("Spend value is more than available");
        }
    }
    */
};

#endif //TINYCHAIN_CPP_TRANSACTION_H
