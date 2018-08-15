//
// Created by kourin on 2018/08/13.
//

#ifndef TINYCHAIN_CPP_LOCATEDBLOCK_H
#define TINYCHAIN_CPP_LOCATEDBLOCK_H

#include <memory>
#include <sstream>

#include "Block.h"

class LocatedBlock {
    public:
        std::shared_ptr<Block> block;
        int height;
        int chainIdx;

        LocatedBlock(std::shared_ptr<Block> block, const int height, const int chainIdx)
        : block(block), height(height), chainIdx(chainIdx) {};

        std::string toString() {
            std::stringstream ss;
            ss << "{";
            ss << "\"block\":" << (this->block != nullptr ? this->block->toString() : "{}") << ",";
            ss << "\"height\":" << this->height << ",";
            ss << "\"chainIdx\"" << this->chainIdx;
            ss << "}";
            return ss.str();
        }
};

#endif //TINYCHAIN_CPP_LOCATEDBLOCK_H
