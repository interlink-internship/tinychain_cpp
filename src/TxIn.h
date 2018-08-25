//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_TXIN_H
#define TINYCHAIN_CPP_TXIN_H


#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <nlohmann/json.hpp>

#include "OutPoint.h"

class TxIn {
    //Inputs to a Transaction.
public:
    //A reference to the output we're spending. This is None for coinbase
    //transactions.
    std::shared_ptr<OutPoint> toSpend;

    // The (signature, pubkey) pair which unlocks the TxOut for spending.
    std::vector<unsigned char> unlockSig;
    std::vector<unsigned char> unlockPk;

    // A sender-defined sequence number which allows us replacement of the txn
    // if desired.
    int sequence;

    TxIn()
    {
        this->toSpend = std::make_shared<OutPoint>();
        this->unlockSig = std::vector<unsigned char>();
        this->unlockPk = std::vector<unsigned char>();
        this->sequence = 0;
    }

    TxIn(std::shared_ptr<OutPoint> toSpend, const std::string unlockSig, const std::string unnlockPk, const int sequence)
            : toSpend(toSpend), sequence(sequence)
    {
        this->unlockSig = std::vector<unsigned char>(unlockSig.begin(), unlockSig.end());
        this->unlockPk = std::vector<unsigned char>(unlockPk.begin(), unlockPk.end());
    }

    TxIn(const TxIn&);

    std::string toString() {
        std::stringstream ss;
        ss << "{";
        ss << "\"toSpend\":" << (this->toSpend != nullptr ? this->toSpend->toString() : "{}") << ",";
        ss << "\"unlockSig\":\"";
        for(const auto c: this->unlockSig) ss << c;
        ss << "\",";
        ss << "\"unlockPk\":\"";
        for(const auto c: this->unlockPk) ss << c;
        ss << "\",";
        ss << "\"sequence\":" << sequence;
        ss << "}";
        return ss.str();
    }

    static std::shared_ptr<TxIn> deserialize(const nlohmann::json& json) {
        auto txin = std::make_shared<TxIn>();

        txin->toSpend = OutPoint::deserialize(json["toSpend"]);

        std::string unlockSig = json["unlockSig"];
        txin->unlockSig = std::vector<unsigned char>(unlockSig.begin(), unlockSig.end());

        std::string unlockPk = json["unlockPk"];
        txin->unlockPk = std::vector<unsigned char>(unlockPk.begin(), unlockSig.end());

        txin->sequence = json["sequence"];

        return txin;
    }
};


#endif //TINYCHAIN_CPP_TXIN_H
