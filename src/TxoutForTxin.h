//
// Created by kourin on 2018/08/13.
//

#ifndef TINYCHAIN_CPP_TXOUTFORTXIN_H
#define TINYCHAIN_CPP_TXOUTFORTXIN_H

#include <memory>

#include "TxOut.h"
#include "Transaction.h"

class TxoutForTxin {
    public:
        std::shared_ptr<TxOut> txout;
        std::shared_ptr<Transaction> tx;
        int txoutIdx;
        bool isCoinbase;
        int height;

        TxoutForTxin(std::shared_ptr<TxOut> txout, std::shared_ptr<Transaction> tx, int txoutIdx, bool isCoinbase, int height)
        : txout(txout), tx(tx), txoutIdx(txoutIdx), isCoinbase(isCoinbase), height(height) {};
};

#endif //TINYCHAIN_CPP_TXOUTFORTXIN_H
