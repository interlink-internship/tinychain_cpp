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
#include <set>
#include <ctime>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>
#include <thread>
#include <future>

#include "Block.h"
#include "Transaction.h"
#include "Params.h"
#include "UnspentTxOut.h"
#include "LocatedBlock.h"
#include "TxoutForTxin.h"
#include "Utility.h"
#include "UtxoSet.h"
#include "Mempool.h"

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

    UtxoSet utxoSet;
    std::vector<std::shared_ptr<Transaction>> orphanTxns;
    Mempool mempool;

    bool mineInterrupt;
    std::mutex mineInterrputMutex;

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
    }

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

    int connectBlock(std::shared_ptr<Block> block) {
        return this->connectBlock(block, false);
    }

    int connectBlock(std::shared_ptr<Block> block, const bool doingReorg) {
           std::lock_guard<std::mutex> lock(this->chainLock);
           //Accept a block and return the chain index we append it to.
           // Only exit early on already seen in active_chain when reorging.
           const auto findBlock = doingReorg
                               ? locateBlock(block->id(), this->activeChain)
                               : locateBlock(block->id());
           if(findBlock->block != nullptr) {
               std::cout << "ignore block already seen: " << block->id() << "\n";
               return -1;
           }

           int chainIdx = 0;
           try {
                block->validate(this->getMedianTimePast(11), this->activeChain.size() > 0, this->activeChainIndex);
           } catch(const Block::BlockValidationException& e) {
               std::cout << "block " << block->id() << " failed validation\n";
               return 0;
           }
            if(block->prevBlockHash == "" && this->activeChain.size() == 0) {
                chainIdx = this->activeChainIndex;
            } else {
                auto prevBlock = this->locateBlock(block->prevBlockHash);
                if(prevBlock->block == nullptr) {
                    std::cout << "prev block " << block->prevBlockHash << " not found in any chain\n";
                    //to_orphen
                }
                if(prevBlock->chainIdx != this->activeChainIndex) {
                    chainIdx = prevBlock->chainIdx;
                } else {
                    chainIdx = prevBlock->chainIdx + 1;
                }
            }
            if(this->getNextWorkRequired(block->prevBlockHash) != block->bits) {
                std::cout << "bits is incorrect\n";
                return -1;
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
                   mempool.deleteUtxoById(tx->id());

                   if(!tx->isCoinbase()) {
                       for(const auto& txin: tx->txins) {
                           utxoSet.rmFromUtxo(txin->toSpend->txid, txin->toSpend->txoutIdx);
                       }
                       int i = 0;
                       for(const auto& txout: tx->txouts) {
                           utxoSet.addToUtxo(txout, tx, i, tx->isCoinbase(), (int)chain.size());
                           ++i;
                       }
                   }
               }
           }

           if( (!doingReorg && this->reorgIfNecessary()) || chainIdx == this->activeChainIndex) {
               this->mineInterrputMutex.lock();
               this->mineInterrupt = true;
               this->mineInterrputMutex.unlock();
               std::cout << "block accepted height=" << (this->activeChain.size()-1) << " txns=" << block->txns.size() << "\n";
           }

           /*
           for(const auto peer: peerHostnames) {
               sendToPeer(block, peer);
           }
            */

           return chainIdx;
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

    /*
     * Proof of Work
     */
    int getNextWorkRequired(const std::string prevBlockHash) {
        // Based on the chain, return the number of difficulty bits the next block
        // must solve.
        if(prevBlockHash == "") {
            return INITIAL_DIFFICULTY_BITS;
        }
        auto locate = this->locateBlock(prevBlockHash);
        if((locate->height + 1) % (int)DIFFICULTY_PERIOD_IN_BLOCKS != 0) {
            return locate->block->bits;
        }

        std::shared_ptr<Block> periodStartBlock = nullptr;
        this->chainLock.lock();
        int idx = std::max(0, locate->height - ((int)DIFFICULTY_PERIOD_IN_BLOCKS - 1));
        periodStartBlock = this->activeChain[idx];
        this->chainLock.unlock();

        auto actualTimeTaken = locate->block->timestamp - periodStartBlock->timestamp;
        if(actualTimeTaken < DIFFICULTY_PERIOD_IN_SECS_TARGET) {
            return locate->block->bits + 1;
        } else if(actualTimeTaken > DIFFICULTY_PERIOD_IN_SECS_TARGET) {
            return locate->block->bits - 1;
        } else {
            return locate->block->bits;
        }
    }

    std::shared_ptr<Block> assembleAndSolveBlock(const std::string payCoinbaseToAddr, std::vector<std::shared_ptr<Transaction>> txns) {
        /*
            Construct a Block by pulling transactions from the mempool, then mine it.
        */
        this->chainLock.lock();
        auto prevBlockHash = (this->activeChain.size() > 0)
                                ? (this->activeChain[this->activeChain.size() - 1]->id())
                                : ("");
        this->chainLock.unlock();

        auto block = std::make_shared<Block>(0, prevBlockHash, "", time(NULL), this->getNextWorkRequired(prevBlockHash), 0, txns);
        if(block->txns.size() == 0) {
            this->selectFromMempool(*block);
        }

        auto fees = this->culculateFees(block);
        //auto myAddress = this->initWallet()[2];
        //auto coinbaseTxn = Transaction::createCoinbase(myAddress, this->getBlockSubsidy() + fees, this->activeChain.size());
        //block->txns.insert(block->txns.begin(), coinbaseTxn);

        block->setMarkleHash();

        /*
        if(block->serialize().size() > MAX_BLOCK_SERIALIZED_SIZE) {
            throw new std::runtime_error("txns specified create a block too large");
        }
         */
        bool isMineSuccess = this->mine(block);
        return (isMineSuccess ? block : nullptr);
    }

    long culculateFees(std::shared_ptr<Block> block) {
        /*
        Given the txns in a Block, subtract the amount of coin output from the
        inputs. This is kept as a reward by the miner.
        */
        long fee = 0;
        for(const auto& txn: block->txns) {
            long spent = 0;
            for(const auto& txin: txn->txins) {
                auto utxo = this->utxoSet.get(txin->toSpend->txid, txin->toSpend->txoutIdx);
                if(utxo != nullptr) {
                    spent += utxo->value;
                    break;
                } else{
                    for(const auto& t: block->txns) {
                        if(t->id() == txin->toSpend->txid) {
                            spent += t->txouts[txin->toSpend->txoutIdx]->value;
                            break;
                        }
                    }
                }
            }

            long sent = 0;
            for(const auto& txout: txn->txouts) {
                sent += txout->value;
            }
            fee += (spent - sent);
        }
        return fee;
    }

    long getBlockSubsidy() {
        long havings = this->activeChain.size() / HALVE_SUBSIDY_AFTER_BLOCKS_NUM;
        return (havings >= 64)
                    ? 0
                    : (50 * BELUSHIS_PER_COIN / (2 << havings));
    }

    bool mine(std::shared_ptr<Block> block) {
        auto start = time(NULL);
        int nonce = 0;
        auto target = boost::multiprecision::uint256_t("1");
        target <<= (256 - block->bits);

        this->mineInterrputMutex.lock();
        this->mineInterrupt = false;
        this->mineInterrputMutex.unlock();

        /*
        std::future<bool> isSuccess
            = std::async(std::launch::async, calcNonce, std::ref(*block), std::ref(nonce), target, std::ref(this->mineInterrputMutex), std::ref(this->mineInterrupt));
        if(isSuccess.get()) {
            block->nonce = nonce;
            auto duration = time(NULL) - start;
            auto khs = (block->nonce / duration) / 1000;
            std::cout << "[mining] block found! " << duration << " s - " << khs << " KH/s - " << block->id() << "\n";
            return true;
        } else {
            return false;
        }
         */
        //
        return false;
        //
    }

    bool mineForever() {
        while(true) {
            /*
            auto myAddress = initWallet()[2];
            auto block = this->assembleAndSolveBlock(myAddress);
            if(block != nullptr) {
                this->connectBlock(block);
                this->saveToDisk();
            }
             */
        }
    }

    bool tryAddToBlock(Block& block, const std::string txid, std::set<std::string>& addedToBlock) {
        if(addedToBlock.count(txid) > 0) {
            return true;
        }
        if(this->mempool.isExist(txid) == 0) {
            std::cout << "!Couldn't find utxo in mempool for " << txid << " in tryAddToBlock\n";
            return false;
        }
        auto tx = this->mempool.mempool[txid];

        // For any txin that can't be found in the main chain, find its
        // transaction in the mempool (if it exists) and add it to the block.
        for(const auto& txin: tx->txins) {
            if(this->utxoSet.get(txin->toSpend->txid, txin->toSpend->txoutIdx) != nullptr) {
                continue;
            }

            auto inMempool = this->mempool.findUtxoInMempool(*txin);
            if(inMempool == nullptr) {
                std::cout << "Couldn't find UTXO for " << txin->toString() << "\n";
                return false;
            }
            auto isSuccess = this->tryAddToBlock(block, inMempool->txid, addedToBlock);
            if(!isSuccess) {
                std::cout << "Couldn't add parent\n";
                return false;
            }
        }

        Block newBlock = block;
        newBlock.txns.push_back(tx);
        /*
        if(newBlock.serialize() < MAX_BLOCK_SERIALIZED_SIZE) {
            std::cout << "added tx " << tx->id() << " to block";
            addedToBlock.insert(txid);
            block = newBlock;
        }
         */
        return true;
    }

    void selectFromMempool(Block& block) {
        std::set<std::string> addToBlock;
        Block newBlock;
        for(const auto& txidTx: this->mempool.mempool) {
            tryAddToBlock(newBlock, txidTx.first, addToBlock);
            /*
            if(newBlock.serialize() < MAX_BLOCK_SERIALIZED_SIZE) {
                block = newBlock;
            } else {
                break;
            }
             */
        }
    }
};

#endif //TINYCHAIN_CPP_TINYCHAIN_H
