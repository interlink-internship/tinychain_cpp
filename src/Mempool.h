//
// Created by kourin on 2018/08/16.
//

#ifndef TINYCHAIN_CPP_MEMPOOL_H
#define TINYCHAIN_CPP_MEMPOOL_H

#include <iostream>
#include <map>
#include <memory>

#include "Transaction.h"
#include "UnspentTxOut.h"

class Mempool {
    public:
        std::map<std::string, std::shared_ptr<Transaction>> mempool;
        Mempool() {}

        std::shared_ptr<UnspentTxOut> findUtxoInMempool(TxIn& txin) {
            auto txid = txin.toSpend->txid;
            auto idx = txin.toSpend->txoutIdx;

            if(this->mempool.count(txid) == 0) {
                std::cout << "Couldn't find utxo in mempool for " << txin.toString() << "\n";
                return nullptr;
            }
            auto txout = this->mempool[txid]->txouts[idx];
            return std::make_shared<UnspentTxOut>(txout->value, txout->toAddress, txid, idx, false, -1);
        }

        void add(std::string key, std::shared_ptr<Transaction> value) {
            this->mempool.insert(std::make_pair(key, value));
        }

        std::shared_ptr<Transaction> get(std::string key) {
            return (this->mempool.count(key) > 0)
                        ? this->mempool[key]
                        : nullptr;
        }

        void deleteUtxoById(const std::string id) {
            this->mempool.erase(id);
        }

        bool isExist(std::string txid) {
            return this->mempool.count(txid) > 0;
        }
};


#endif //TINYCHAIN_CPP_MEMPOOL_H
