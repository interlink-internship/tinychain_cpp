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
#include <nlohmann/json.hpp>

#include "Utility.h"
#include "Params.h"
#include "OutPoint.h"
#include "TxIn.h"
#include "TxOut.h"
#include "UnspentTxOut.h"

bool validateSignatureForSpend(std::shared_ptr<TxIn>, std::shared_ptr<UnspentTxOut>, std::vector<std::shared_ptr<TxOut>>&);

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

    static std::shared_ptr<Transaction> deserialize(const nlohmann::json& json) {
        auto tx = std::make_shared<Transaction>();

        auto txinSize = json["txins"].size();
        tx->txins.resize(txinSize);
        for(size_t i = 0; i< txinSize; ++i) {
            tx->txins[i] = TxIn::deserialize(json["txins"][i]);
        }

        auto txoutSize = json["txouts"].size();
        tx->txouts.resize(txoutSize);
        for(size_t i = 0; i < txoutSize; ++i) {
            tx->txouts[i] = TxOut::deserialize(json["txouts"][i]);
        }

        tx->locktime = json["locktime"];
        return tx;
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

    // exception using validation
    class TransactionValidationException : public std::runtime_error {
        public:
            bool isOrphen;
            TransactionValidationException(const char *_Message)
                    : isOrphen(false), runtime_error(_Message) {};
            TransactionValidationException(const char *_Message, const bool isOrphen)
                    : isOrphen(isOrphen), runtime_error(_Message) {};
    };

    void validate(std::map<OutPoint, UnspentTxOut>& utxoSet, std::map<std::string, std::shared_ptr<Transaction>>& mempool, const int currentHeight) {
        auto slidingBlocks = std::vector<std::shared_ptr<Transaction>>();
        return this->validate(utxoSet, mempool, currentHeight, slidingBlocks, false, true);
    }

    void validate(std::map<OutPoint, UnspentTxOut>& utxoSet, std::map<std::string, std::shared_ptr<Transaction>>& mempool, const int currentHeight, std::vector<std::shared_ptr<Transaction>>& slidingsInBlock) {
        return this->validate(utxoSet, mempool, currentHeight, slidingsInBlock, false, true);
    }

    void validate(std::map<OutPoint, UnspentTxOut>& utxoSet, std::map<std::string, std::shared_ptr<Transaction>>& mempool, const int currentHeight, std::vector<std::shared_ptr<Transaction>>& slidingsInBlock, const bool allowUtxoFromMempool) {
        return this->validate(utxoSet, mempool, currentHeight, slidingsInBlock, false, allowUtxoFromMempool);
    }

    void validate(std::map<OutPoint, UnspentTxOut>& utxoSet, std::map<std::string, std::shared_ptr<Transaction>>& mempool, const int currentHeight, std::vector<std::shared_ptr<Transaction>>& slidingsInBlock, const bool asCoinbase, const bool allowUtxoFromMempool) {

        // Validate a single transaction. Used in various contexts, so the
        // parameters facilitate different uses.
        this->validateBasics(asCoinbase);

        long availableToSpend = 0;
        long i = 0;
        for(auto& txin: this->txins) {
            /* isSame
             * auto utxo = utxoSet.get(txin->toSpend->txid, txin->toSpend->txoutIdx); */
            const auto outpoint = OutPoint(txin->toSpend->txid, txin->toSpend->txoutIdx);
            auto utxo = utxoSet.count(outpoint) > 0 ? std::shared_ptr<UnspentTxOut>(&utxoSet[outpoint]) : nullptr;

            if(utxo == nullptr && slidingsInBlock.size() > 0) {
                utxo = this->findUtxoInList(txin, slidingsInBlock);
            }
            if(utxo == nullptr && allowUtxoFromMempool && mempool.count(txin->toSpend->txid) > 0) {
                /* isSame
                 * utxo = mempool.findUtxo(txin);
                 * */
                auto txout = mempool[txin->toSpend->txid]->txouts[txin->toSpend->txoutIdx];
                utxo = std::make_shared<UnspentTxOut>(txout->value, txout->toAddress, txin->toSpend->txid, txin->toSpend->txoutIdx, false, -1);
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
            } catch(std::runtime_error e) {
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
};

#endif //TINYCHAIN_CPP_TRANSACTION_H
