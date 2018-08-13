//
// Created by kourin on 2018/08/12.
//

#ifndef TINYCHAIN_CPP_TINYCHAIN_H
#define TINYCHAIN_CPP_TINYCHAIN_H


#include <vector>
#include <mutex>
#include <utility>

#include "Block.h"
#include "Transaction.h"
#include "Params.h"
#include "UnspentTxOut.h"
#include "LocatedBlock.h"
#include "TxoutForTxin.h"

class TinyChain {
    public:
        std::shared_ptr<Block> genesisBlock;

        // The highest proof-of-work, valid blockchain.
        //
        // #realname chainActive
        std::vector<std::shared_ptr<Block>> activeChain;

        // Branches off of the main chain.
        std::vector<std::vector<std::shared_ptr<Block>>> sideBranches;

        // Synchronize access to the active chain and side branches.
        std::mutex chainLock;

        int activeChainIndex;

        TinyChain() {
            //genesis block
            int version = 0;
            std::string prevBlockHash = "";
            std::string markleHash = "7118894203235a955a908c0abfc6d8fe6edec47b0a04ce1bf7263da3b4366d22";
            int timestamp = 1501821412;
            int bits = 24;
            int nonce = 10126761;

            auto txin = std::make_shared<TxIn>(std::make_unique<OutPoint>(), "0", "", 0);
            auto txout = std::make_shared<TxOut>(5000000000, "143UVyz7ooiAv1pMqbwPPpnH4BV9ifJGFF");
            auto tx = std::make_shared<Transaction>(std::move(txin), std::move(txout));
            std::vector<std::shared_ptr<Transaction>> txns;
            txns.push_back(tx);
            this->genesisBlock = std::make_shared<Block>(version, prevBlockHash, markleHash, timestamp, bits, nonce, txns);

            //active chain
            this->activeChain.reserve(RESERVE_BLOCK_SIZE_OF_CHAIN);
            this->activeChain.push_back(this->genesisBlock);

            this->activeChainIndex = 0;
        }

        int getCurrentHight() {
            std::lock_guard<std::mutex> lock(this->chainLock);
            return this->activeChain.size();
        }


        std::shared_ptr<LocatedBlock> locateBlock(const std::string blockHash, std::vector<std::shared_ptr<Block>> chain) {
            std::lock_guard<std::mutex> lock(this->chainLock);
            int chainIdx = 0, height = 0;
            if(chain.size() != 0) {
                for(auto block: chain) {
                    if(block->id() == blockHash) {
                        return std::make_shared<LocatedBlock>(block, height, chainIdx);
                    }
                    ++height;
                }

            } else {
                for(auto block: this->activeChain) {
                    if(block->id() == blockHash) {
                        return std::make_shared<LocatedBlock>(block, height, chainIdx);
                    }
                }
                ++chainIdx;

                for(auto ch: this->sideBranches) {
                    height = 0;
                    for(const auto& block: ch) {
                        if(block->id() == blockHash) {
                            return std::make_shared<LocatedBlock>(block, height, chainIdx);
                        }
                    }
                    ++chainIdx;
                }
            }
            return std::make_shared<LocatedBlock>(nullptr, 0, 0);
        }

        std::shared_ptr<TxoutForTxin> findTxoutForTxin(std::shared_ptr<TxIn> txin, std::vector<std::shared_ptr<Block>> chain) {
            std::lock_guard<std::mutex> lock(this->chainLock);
            auto txid = txin->toSpend->txid;
            auto txoutIdx = txin->toSpend->txout_idx;

            int height = 0;
            for(const auto& block: chain) {
                for(const auto tx: block->txns) {
                    if(tx->id() == txid) {
                        auto txout = tx->txouts[txoutIdx];
                        return std::make_shared<TxoutForTxin>(txout, tx, txoutIdx, tx->isCoinbase(), height);
                    }
                }
                ++height;
            }
        }
};



#endif //TINYCHAIN_CPP_TINYCHAIN_H
