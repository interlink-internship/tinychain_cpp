//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_TXOUT_H
#define TINYCHAIN_CPP_TXOUT_H


#include <string>
#include <cereal/types/vector.hpp>

class TxOut {
public:
    int value;
    std::string toAddr;

    TxOut(): value(0), toAddr(""){};
    TxOut(const int value, const std::string toAddr):value(value), toAddr(toAddr){};

    template<class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(value), CEREAL_NVP(toAddr));
    }
};


#endif //TINYCHAIN_CPP_TXOUT_H
