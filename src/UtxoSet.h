//
// Created by kourin on 2018/08/15.
//

#ifndef TINYCHAIN_CPP_UTXOSET_H
#define TINYCHAIN_CPP_UTXOSET_H

#include <iostream>
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

        std::shared_ptr<UnspentTxOut> get(const std::string txid, const int txoutIdx) {
            const auto outPoint = OutPoint(txid, txoutIdx);
            return (this->utxoSet.count(outPoint) > 0)
                        ? std::shared_ptr<UnspentTxOut>(&this->utxoSet[outPoint])
                        : nullptr;
        }
};

#endif //TINYCHAIN_CPP_UTXOSET_H
