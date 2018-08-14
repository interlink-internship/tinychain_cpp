#ifndef TINYCHAIN_CPP_DISCONNECTBLOCK_H
#define TINYCHAIN_CPP_DISCONNECTBLOCK_H

#include <sstream>
#include <memory>

#include "Block.h"



class DisconnectBlock {
  public:
  std::shared_ptr<Block> block;

  int chainIdx;

  DisconnectBlock(std::shared_ptr<Block> block, const int chainIdx)
    : Block(block), chainIdx(chainIdx) {};


  std::string toString() {
    std::stringstream ss;
    ss << "{";
    ss << "\"block\":" << (this->block != nullptr ? this->block->toString() : "{}") << ",";
    ss << "\"chainIdx\":" << this->chainIdx;
    ss << "}";
    return ss.str();
  }


};

#endif //TINYCHAIN_CPP_DISCONNECTBLOCK_H
