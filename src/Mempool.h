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
        Mempool() {
        }

        void addTxnToMempool(std::shared_ptr<Transaction> txn) {
            if(this->mempool.count(txn->id()) > 0) {
                std::cout << "txn " << txn->id() << " already seen\n";
                return;
            }

            /*
            try {
                txn->validate(this->utxoSet, this->mempool, this->activeChain.size());
                std::cout << "txn " << txn->id() << " added to mempool\n";
                mempool.insert(std::make_pair(txn->id(), txn));

                this->bloadCast();
            } catch(const Transaction::TransactionValidationException& e) {
                if(e.isOrphen) {
                    std::cout << "txn " << txn.id() << " submitted as orphan\n";
                    this->orphanTxns.push_back(txn);
                } else {
                    std::cout << "txn rejected\n";
                }
            }
             */
        }

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

        void deleteUtxoById(const std::string id) {
            this->mempool.erase(id);
        }

        bool isExist(std::string txid) {
            return this->mempool.count(txid) > 0;
        }
};


#endif //TINYCHAIN_CPP_MEMPOOL_H
