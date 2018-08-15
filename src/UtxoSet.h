//
// Created by kourin on 2018/08/15.
//

#ifndef TINYCHAIN_CPP_UTXOSET_H
#define TINYCHAIN_CPP_UTXOSET_H

#include <map>
#include <memory>
#include <utility>

#include "OutPoint.h"
#include "UnspentTxOut.h"
#include "TxOut.h"
#include "Transaction.h"

class UtxoSet {
    public:
        std::map<OutPoint, UnspentTxOut> utxoSet;

        UtxoSet() {
        }

        void addToUtxo(std::shared_ptr<TxOut> txout, std::shared_ptr<Transaction> tx, const int idx, const int isCoinbase, const int height) {
            auto utxo = UnspentTxOut(txout->value, txout->toAddress, tx->id(), idx, isCoinbase, height);
            std::cout << "adding tx outpoint " << utxo.toString() << " to utxo_set\n";
            if(utxoSet.count(utxo.outpoint()) > 0) {
                utxoSet[utxo.outpoint()] = utxo;
            } else {
                utxoSet.insert(std::make_pair(utxo.outpoint(), utxo));
            }
        }

        void rmFromUtxo(const std::string txid, const int txoutIdx) {
            utxoSet.erase(OutPoint(txid, txoutIdx));
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

#endif //TINYCHAIN_CPP_UTXOSET_H
