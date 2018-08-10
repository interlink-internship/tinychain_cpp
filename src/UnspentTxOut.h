//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_UNSPENTTXOUT_H
#define TINYCHAIN_CPP_UNSPENTTXOUT_H

#include <string>
#include <cereal/types/vector.hpp>

class UnspentTxOut {
public:
    int value;
    std::string toAddr;

    //The ID of the transaction this output belongs to.
    std::string txid;
    int txout_idx;

    //Did this TxOut from from a coinbase transaction?
    bool isCoinbase;

    // The blockchain height this TxOut was included in the chain.
    int height;

    UnspentTxOut(): value(0), toAddr(""), txid(""), txout_idx(0), isCoinbase(false), height(0){};

    template<class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(value), CEREAL_NVP(toAddr), CEREAL_NVP(txid), CEREAL_NVP(txout_idx), CEREAL_NVP(isCoinbase), CEREAL_NVP(height));
    }
};


#endif //TINYCHAIN_CPP_UNSPENTTXOUT_H
