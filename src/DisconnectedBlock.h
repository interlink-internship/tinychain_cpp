#ifndef TINYCHAIN_CPP_DISCONNECTBLOCK_H
#define TINYCHAIN_CPP_DISCONNECTBLOCK_H

#include "Block.h"

class DisconnectBlock {
 public:
  std::shared_ptr<Block> block;

  int chainIdx;

  DisconnectBlock(std::shared_ptr<Block> block, const int chainIdx)
    : block(block), chainIdx(chainIdx) {};
};

#endif //TINYCHAIN_CPP_LOCATEBLOCK_H
