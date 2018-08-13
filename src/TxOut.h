//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_TXOUT_H
#define TINYCHAIN_CPP_TXOUT_H


#include <string>
#include <sstream>
#include <cereal/types/vector.hpp>

class TxOut {
public:
    int value;
    std::string toAddress;

    TxOut(): value(0), toAddress(""){};
    TxOut(const int value, const std::string toAddress):value(value), toAddress(toAddress){};
    TxOut(const TxOut&);

    template<class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(value), CEREAL_NVP(toAddress));
    }

    std::string toString() {
        std::stringstream ss;
        ss << "{";
        ss << "\"value\":" << this->value << ",";
        ss << "\"toAddress\":\"" << this->toAddress << "\"";
        ss << "}";
        return ss.str();
    }
};


#endif //TINYCHAIN_CPP_TXOUT_H


