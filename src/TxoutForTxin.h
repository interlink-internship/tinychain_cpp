//
// Created by kourin on 2018/08/13.
//

#ifndef TINYCHAIN_CPP_TXOUTFORTXIN_H
#define TINYCHAIN_CPP_TXOUTFORTXIN_H

#include <memory>
#include <sstream>

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

        std::string toString() {
            std::stringstream ss;
            ss << "{";
            ss << "\"txout\":" << (this->txout != nullptr ? this->txout->toString() : "{}") << ",";
            ss << "\"tx\":" << (this->tx != nullptr ? this->tx->toString() : "{}") << ",";
            ss << "\"txoutIdx\":" << this->txoutIdx << ",";
            ss << "\"isCoinbase\":" << (this->isCoinbase ? "true" : "false") << ",";
            ss << "\"height\":" << this->height;
            ss << "}";
            return ss.str();
        }
};

#endif //TINYCHAIN_CPP_TXOUTFORTXIN_H
