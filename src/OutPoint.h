//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_OUTPOINT_H
#define TINYCHAIN_CPP_OUTPOINT_H

#include <string>
#include <sstream>
#include <memory>
#include <nlohmann/json.hpp>

class OutPoint {
public:
    std::string txid;
    int txoutIdx;

    OutPoint() : txid(""), txoutIdx(0){};
    OutPoint(const std::string& txid, const int txoutIdx): txid(txid), txoutIdx(txoutIdx) {}

    OutPoint(const OutPoint& outpoint) {
        this->txid = outpoint.txid;
        this->txoutIdx = outpoint.txoutIdx;
    }

    std::string toString() {
        std::stringstream ss;
        ss << "{";
        ss << "\"txid\":\"" << this->txid << "\",";
        ss << "\"txoutIdx\":" << this->txoutIdx;
        ss << "}";
        return ss.str();
    }

    static std::shared_ptr<OutPoint> deserialize(const nlohmann::json& json) {
        auto outpoint = std::make_shared<OutPoint>();
        outpoint->txid = json["txid"];
        outpoint->txoutIdx = json["txoutIdx"];
        return outpoint;
    }

    bool operator<(const OutPoint &right) const {return this->txid < right.txid;};
};

#endif //TINYCHAIN_CPP_OUTPOINT_H
