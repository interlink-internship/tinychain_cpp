//
// Created by kourin on 2018/08/10.
//

#ifndef TINYCHAIN_CPP_OUTPOINT_H
#define TINYCHAIN_CPP_OUTPOINT_H

#include <string>
#include <cereal/types/vector.hpp>

class OutPoint {
public:
    std::string txid;
    int txout_idx;

    OutPoint() : txid(""), txout_idx(0){};
    OutPoint(const std::string& txid, const int txout_idx): txid(txid), txout_idx(txout_idx) {
    }

    template<class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(txid), CEREAL_NVP(txout_idx));
    }
};

#endif //TINYCHAIN_CPP_OUTPOINT_H
