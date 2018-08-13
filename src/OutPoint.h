//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_OUTPOINT_H
#define TINYCHAIN_CPP_OUTPOINT_H

#include <string>
#include <sstream>
#include <cereal/types/vector.hpp>

class OutPoint {
public:
    std::string txid;
    int txoutIdx;

    OutPoint() : txid(""), txoutIdx(0){};
    OutPoint(const std::string& txid, const int txoutIdx): txid(txid), txoutIdx(txoutIdx) {}

    template<class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(txid), CEREAL_NVP(txoutIdx));
    }

    OutPoint(const OutPoint&);

    std::string toString() {
        std::stringstream ss;
        ss << "{";
        ss << "\"txid\":\"" << this->txid << "\",";
        ss << "\"txoutIdx\":" << this->txoutIdx;
        ss << "}";
        return ss.str();
    }
};

#endif //TINYCHAIN_CPP_OUTPOINT_H
