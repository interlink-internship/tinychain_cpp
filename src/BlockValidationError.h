//
// Created by kourin on 2018/08/13.
//

#ifndef TINYCHAIN_CPP_BLOCKVALIDATIONERROR_H
#define TINYCHAIN_CPP_BLOCKVALIDATIONERROR_H

#include <exception>
#include <memory>

#include "Block.h"

class BlockValidationError : public std::bad_exception {
    public:
        std::shared_ptr<Block> toOrpan;
};

#endif //TINYCHAIN_CPP_BLOCKVALIDATIONERROR_H
