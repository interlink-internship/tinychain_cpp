//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_TXOUT_H
#define TINYCHAIN_CPP_TXOUT_H


#include <string>
#include <sstream>
#include <memory>
#include <nlohmann/json.hpp>

class TxOut {
public:
    int value;
    std::string toAddress;

    TxOut(): value(0), toAddress(""){};
    TxOut(const int value, const std::string toAddress):value(value), toAddress(toAddress){};
    TxOut(const TxOut&);

    std::string toString() {
        std::stringstream ss;
        ss << "{";
        ss << "\"value\":" << this->value << ",";
        ss << "\"toAddress\":\"" << this->toAddress << "\"";
        ss << "}";
        return ss.str();
    }

    static std::shared_ptr<TxOut> deserialize(const nlohmann::json& json) {
        auto txout = std::make_shared<TxOut>();
        txout->value = json["value"];
        txout->toAddress = json["toAddress"];
        return txout;
    }
};


#endif //TINYCHAIN_CPP_TXOUT_H


