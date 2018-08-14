//
// Created by kourin on 2018/08/12.
//

#ifndef TINYCHAIN_CPP_TINYCHAIN_H
#define TINYCHAIN_CPP_TINYCHAIN_H


#include <vector>
#include <mutex>
#include <utility>
#include <cassert>
#include <algorithm>

#include "Block.h"
#include "Transaction.h"
#include "Params.h"
#include "UnspentTxOut.h"
#include "LocatedBlock.h"
#include "TxoutForTxin.h"
#include "Utility.h"

class TinyChain {
    public:

    using BlockChain = std::vector<std::shared_ptr<Block>>;

    std::shared_ptr<Block> genesisBlock;

    // The highest proof-of-work, valid blockchain.
    //
    // #realname chainActive
    BlockChain activeChain;

    // Branches off of the main chain.
    std::vector<BlockChain> sideBranches;

    std::vector<std::shared_ptr<Block>> orphanBlocks;

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

    // return active shain length
    int getCurrentHeight() {
        std::lock_guard<std::mutex> lock(this->chainLock);
        return this->activeChain.size();
    }

    std::shared_ptr<LocatedBlock> locateBlock(const std::string blockHash) {
        auto locate = this->locateBlock(blockHash, this->activeChain);
        if(locate->block == nullptr) {
            for(const auto& chain: this->sideBranches) {
                locate = this->locateBlock(blockHash, chain);
                if(locate->block != nullptr) {
                    break;
                }
            }
        }
        return locate;
    }

    // find block in a specified chain or both of active chain and side branches
    std::shared_ptr<LocatedBlock> locateBlock(const std::string blockHash, TinyChain::BlockChain chain) {
        std::lock_guard<std::mutex> lock(this->chainLock);
        int chainIdx = 0, height = 0;
        for(auto block: chain) {
            if(block->id() == blockHash) {
                return std::make_shared<LocatedBlock>(block, height, chainIdx);
            }
            ++height;
        }
        return std::make_shared<LocatedBlock>(nullptr, 0, 0);
    }


    int connectBlock(std::shared_ptr<Block> block, const bool doingReorg) {

           std::lock_guard<std::mutex> lock(this->chainLock);
           //Accept a block and return the chain index we append it to.
           // Only exit early on already seen in active_chain when reorging.
           const auto res = doingReorg
                               ? locateBlock(block->id(), activeChain)
                               : locateBlock(block->id());
           if(res->block != nullptr) {
               std::cout << "ignore block already seen: " << block->id() << "\n";
               return -1;
           }


           int chainIdx = 0;
           try {
                block->validateBlock(this->getMedianTimePast(11), this->activeChain.size() > 0, this->activeChainIndex);
           } catch(const Block::BlockValidationException& e) {
               std::cout << "block " << block->id() << " failed validation\n";
               return 0;
           }
            if(block->prevBlockHash == "" && this->activeChain.size() == 0) {
                chainIdx = this->activeChainIndex;
            } else {
                auto prevBlock = this->locateBlock(block->prevBlockHash);

            }


           // If `validate_block()` returned a non-existent chain index, we're
           // creating a new side branch.
           if(chainIdx != this->activeChainIndex && this->sideBranches.size() < chainIdx) {
               std::cout << "creating a new side branch (idx " << chainIdx << ") for block " << block->id() << "\n";
               this->sideBranches.push_back(BlockChain());
           }

           std::cout << "connecting block " << block->id() << " to chain " << chainIdx << "\n";
           auto chain = (chainIdx == this->activeChainIndex)
                           ? this->activeChain
                           : this->sideBranches[chainIdx - 1];
           chain.push_back(block);

           // If we added to the active chain, perform upkeep on utxo_set and mempool.
           if(chainIdx == this->activeChainIndex) {
               for(const auto& tx: block->txns) {
                   //mempool.pop(tx->id(), nullptr);

                   if(!tx->isCoinbase()) {
                       for(const auto& txin: tx->txins) {
                           //rmFromUtxo(txin->toSpend);
                       }
                       int i = 0;
                       for(const auto& txout: tx->txouts) {
                           //addToUxto(txout, tx, i, tx->isCoinbase(), chain.size());
                           ++i;
                       }
                   }
               }
           }

           if( (!doingReorg && this->reorgIfNecessary()) || chainIdx == this->activeChainIndex) {
               //mine_interrupt.set();
               std::cout << "block accepted height=" << (this->activeChain.size()-1) << " txns=" << block->txns.size() << "\n";
           }

           /*
           for(const auto peer: peerHostnames) {
               sendToPeer(block, peer);
           }
            */

           return chainIdx;

        return 0;
    }


    std::shared_ptr<Block> disconnectBlock(std::shared_ptr<Block> block, BlockChain chain) {
        BlockChain useChain = (chain.size() > 0) ? chain : this->activeChain;
        assert(block == *useChain.rbegin());

        for(const auto& tx: block->txns) {
            //mempool[tx->id()] = tx;

            for(const auto& txin: tx->txins) {
                if(txin->toSpend->txid != "") {
                    auto txoutTxin = findTxoutForTxin(txin, useChain);
                    //addToUtxo(*txoutTxin);
                }
            }
            int len = tx->txouts.size();
            for(int i=0; i<len; ++i) {
                //rmFromUtxo(tx->id(), i);
            }
        }
        std::cout << "block " << block->id() << " disconnected\n";
        auto last = (*useChain.rbegin());
        useChain.pop_back();
        return last;
    }


    std::shared_ptr<TxoutForTxin> findTxoutForTxin(std::shared_ptr<TxIn> txin, BlockChain chain) {
        std::lock_guard<std::mutex> lock(this->chainLock);
        auto txid = txin->toSpend->txid;
        auto txoutIdx = txin->toSpend->txoutIdx;

        int height = 0;
        for(const auto& block: chain) {
            for(const auto& tx: block->txns) {
                if(tx->id() == txid) {
                    auto txout = tx->txouts[txoutIdx];
                    return std::make_shared<TxoutForTxin>(txout, tx, txoutIdx, tx->isCoinbase(), height);
                }
            }
            ++height;
        }
        return nullptr;
    }

    bool reorgIfNecessary() {
        std::lock_guard<std::mutex> lock(this->chainLock);
        auto reorged = false;
        //2次元配列コピー
        const auto frozenSideBranches = this->sideBranches;
        //# TODO should probably be using `chainwork` for the basis of
        //# comparison here.

        const int numSideBranches = frozenSideBranches.size();
        for(int i = 0; i < numSideBranches; ++i) {
            const auto chain = frozenSideBranches[i];
            const auto forkBlock = this->locateBlock(chain[0]->prevBlockHash, this->activeChain);
            const auto activeHeight = this->activeChain.size();
            const auto branchHeight = chain.size() + forkBlock->height;

            if(branchHeight > activeHeight) {
                std::cout << "attempting reorg of idx " << (i+1) << " to active_chain: ";
                std::cout << "new height of " << branchHeight << "(vs. " << activeHeight << ")\n";
                reorged |= tryReorg(chain, i+1, forkBlock->height);
            }
        }
        return reorged;
    }

    bool tryReorg(BlockChain branch, const int branchIdx, const int forkIdx) {
        // Use the global keyword so that we can actually swap out the reference
        // in case of a reorg.
        auto forkBlock = this->activeChain[forkIdx];
        BlockChain oldActive;

        auto it = activeChain.end() - 1, _BEGIN = activeChain.begin();
        while(it != _BEGIN && (*it)->id() != forkBlock->id()) {
            auto res = disconnectBlock(*it, BlockChain());
            oldActive.push_back(res);
            --it;
        }
        std::reverse(oldActive.begin(), oldActive.end());

        assert(branch[0]->prevBlockHash == (*activeChain.rbegin())->id());

        for(const auto& block: branch) {
            int connectedIdx = this->connectBlock(block, true);
            if(connectedIdx != this->activeChainIndex) {
                //rollback
                std::cout << "reorg of idx " << branchIdx << " to active_chain failed\n";
                auto it = activeChain.end() - 1, _BEGIN = activeChain.begin();
                while(it != _BEGIN && (*it)->id() != forkBlock->id()) {
                    auto res = disconnectBlock(*it, BlockChain());
                    --it;
                }
                for(const auto& block: oldActive) {
                    int idx = connectBlock(block, true);
                    assert(idx == this->activeChainIndex);
                }
                return false;
            }
        }

        this->sideBranches.erase(this->sideBranches.begin() + branchIdx - 1);
        this->sideBranches.push_back(oldActive);

        std::cout << "chain reorg! New height: " << activeChain.size() << ", tip:" << (*activeChain.end())->id() << "\n";
        return true;
    }


    int getMedianTimePast(const int numLastBlocks) {
        if(this->activeChain.size() == 0) {
            return 0;
        } else {
            int numN = std::min(numLastBlocks, (int)this->activeChain.size());
            int mid = numN / 2;
            return this->activeChain[this->activeChain.size() - 1 - mid]->timestamp;
        }
    }
};

#endif //TINYCHAIN_CPP_TINYCHAIN_H
