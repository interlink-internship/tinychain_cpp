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
#include <fstream>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread/thread.hpp>
#include <boost/coroutine/stack_traits.hpp>
#include <boost/coroutine/stack_context.hpp>
#include <boost/coroutine/stack_allocator.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread/thread.hpp>
#include <boost/coroutine/stack_traits.hpp>
#include <boost/coroutine/stack_context.hpp>
#include <boost/coroutine/stack_allocator.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <random>
#include <utility>
#include <functional>

#include "Block.h"
#include "Transaction.h"
#include "Params.h"
#include "UnspentTxOut.h"
#include "LocatedBlock.h"
#include "TxoutForTxin.h"
#include "Utility.h"
#include "UtxoSet.h"
#include "Mempool.h"
#include "Server.h"
#include "Client.h"
#include "Message.h"

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

    std::string chainPath;
    std::string walletPath;

    std::vector<unsigned char> privateKey;
    std::vector<unsigned char> publicKey;
    std::string myAddress;

    std::set<std::pair<std::string, uint16_t>> peers;
    uint16_t port;

    boost::asio::io_service& ioService;

    std::shared_ptr<Server> server;
    std::shared_ptr<Client> client;

    std::random_device random;

    TinyChain(boost::asio::io_service& ioService, const std::string chainPath, const std::string walletPath, const std::set<std::pair<std::string, uint16_t>>& peers, const uint16_t port)
        : ioService(ioService)
    {
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

        this->chainPath = chainPath;
        this->walletPath = walletPath;
        this->peers = peers;
        this->port = port;
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
               block->validate(this->utxoSet.utxoSet, this->mempool.mempool, this->activeChain.size(), this->getMedianTimePast(11));
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
                    this->orphanBlocks.push_back(block);
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

           for(const auto peer: this->peers) {
               sendToPeer(std::dynamic_pointer_cast<AbstractMessage>(std::make_shared<PostBlock>(block)), peer.first, peer.second);
           }
           return chainIdx;
    }


    std::shared_ptr<Block> disconnectBlock(std::shared_ptr<Block> block, BlockChain chain) {
        BlockChain useChain = (chain.size() > 0) ? chain : this->activeChain;
        assert(block == *useChain.rbegin());

        for(const auto& tx: block->txns) {
            this->mempool.add(tx->id(), tx);

            for(const auto& txin: tx->txins) {
                if(txin->toSpend->txid != "") {
                    auto txoutTxin = findTxoutForTxin(txin, useChain);
                    utxoSet.addToUtxo(txoutTxin->txout, txoutTxin->tx, txoutTxin->txoutIdx, txoutTxin->isCoinbase, txoutTxin->height);
                }
            }
            int len = tx->txouts.size();
            for(int i=0; i<len; ++i) {
                utxoSet.rmFromUtxo(tx->id(), i);
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
        auto coinbaseTxn = Transaction::createCoinbase(this->myAddress, this->getBlockSubsidy() + fees, this->activeChain.size());
        block->txns.insert(block->txns.begin(), coinbaseTxn);

        block->setMarkleHash();

        if(block->serialize().size() > MAX_BLOCK_SERIALIZED_SIZE) {
            throw new std::runtime_error("txns specified create a block too large");
        }
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

        auto mining = [](Block& block, int& nonce, const boost::multiprecision::uint256_t target, std::mutex& mineInterrupt, bool& isMineInterrupt)->bool{
            while(true) {
                auto header = block.header(nonce);
                std::vector<unsigned char> bytes;
                hexStringToBytes(header, bytes);
                unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
                bytesSha256(bytes.data(), bytes.size(), sha256Hash);
                bytesSha256(sha256Hash, SHA256_DIGEST_LENGTH, sha256Hash);

                boost::multiprecision::uint256_t value = 0;
                for(int i=0; i<SHA256_DIGEST_LENGTH; ++i) {
                    value *= 256;
                    value += sha256Hash[i];
                }
                if(value < target) {
                    break;
                }

                ++nonce;
                if(nonce % 100000 == 0) {
                    mineInterrupt.lock();
                    bool isMineInterrupt = isMineInterrupt;
                    mineInterrupt.unlock();

                    if(isMineInterrupt) {
                        std::cout << "[mining] interrupted" << std::endl;
                        mineInterrupt.lock();
                        bool isMineInterrupt = false;
                        mineInterrupt.unlock();
                        return false;
                    }
                }
            }
            return true;
        };

        std::future<bool> isSuccess
            = std::async(std::launch::async, mining, std::ref(*block), std::ref(nonce), target, std::ref(this->mineInterrputMutex), std::ref(this->mineInterrupt));
        if(isSuccess.get()) {
            block->nonce = nonce;
            auto duration = time(NULL) - start;
            auto khs = (block->nonce / duration) / 1000;
            std::cout << "[mining] block found! " << duration << " s - " << khs << " KH/s - " << block->id() << "\n";
            return true;
        } else {
            return false;
        }
    }

    bool mineForever() {
        while(true) {
            /*
            auto block = this->assembleAndSolveBlock(this->myAddress);
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
        if(newBlock.serialize().size() < MAX_BLOCK_SERIALIZED_SIZE) {
            std::cout << "added tx " << tx->id() << " to block";
            addedToBlock.insert(txid);
            block = newBlock;
        }
        return true;
    }

    void selectFromMempool(Block& block) {
        std::set<std::string> addToBlock;
        Block newBlock;
        for(const auto& txidTx: this->mempool.mempool) {
            tryAddToBlock(newBlock, txidTx.first, addToBlock);
            if(newBlock.serialize().size() < MAX_BLOCK_SERIALIZED_SIZE) {
                block = newBlock;
            } else {
                break;
            }
        }
    }

    void loadFromDisk() {
        std::cout << "Load From Disk: " << this->chainPath << std::endl;
        auto ifs = std::ifstream(this->chainPath, std::ios::binary);
        if(this->chainPath.empty() || !ifs.is_open()) {
            std::cout << "File Not Exist: " << this->chainPath << std::endl;
            ifs.close();
            return;
        }

        ifs.seekg(0,std::ios::end);
        const size_t fileSize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        if(fileSize < 3) {
            std::cout << "File Size is too small: " << fileSize << std::endl;
        }

        auto headerSignedByte = new char[HEADER_SIZE];
        auto headerUnsignedByte = new unsigned char[HEADER_SIZE];
        ifs.read(headerSignedByte, HEADER_SIZE);
        for(size_t i=0; i<HEADER_SIZE; ++i) {
            headerUnsignedByte[i] = (unsigned char)headerUnsignedByte[i];
        }
        const auto readSize = bytesToInteger<size_t>(4, headerUnsignedByte);
        std::cout << "Read Size: " << readSize << std::endl;

        auto bodyBytes = new char[readSize];
        ifs.read(bodyBytes, readSize);
        std::string body = std::string(bodyBytes, bodyBytes + readSize);
        std::cout << "Read Success: " << body.size() << std::endl;

        auto json = nlohmann::json::parse(body);
        if(!json.is_array()) {
            std::cout << "Error: File Top Must Be Array" << std::endl;
            return;
        }

        const size_t numBlocks = json.size();
        std::vector<std::shared_ptr<Block>> newBlocks(numBlocks);
        for(size_t i = 0; i < numBlocks; ++i) {
            newBlocks[i] = Block::deserialize(json[i]);
        }

        for(auto& block: newBlocks) {
            connectBlock(block);
        }

        delete headerSignedByte, headerUnsignedByte;
        ifs.close();
        return;
    }

    void saveToDisk() {
        std::ofstream ofs(this->chainPath);

        std::stringstream ss;
        {
            ss << "[";
            int cnt = 0;
            for(const auto& chain: this->activeChain) {
                ss << (cnt++ > 0 ? "," : "");
                ss << chain->serialize();
            }
            ss << "]";
        }

        auto data = ss.str();
        auto size = data.size();
        std::vector<unsigned char> headerBytesVector;
        integerToBytes(size, HEADER_SIZE, headerBytesVector);
        auto headerBytes = new char[HEADER_SIZE];
        for(size_t i = 0; i < HEADER_SIZE; ++i) {
            headerBytes[i] = headerBytesVector[i];
        }
        ofs.write(headerBytes, HEADER_SIZE);

        ofs.write(data.c_str(), data.size());
        ofs.close();
        return;
    }

    bool isWalletExist() {
        if(this->walletPath == "") {
            std::cout << "Wallet Path is empty" << std::endl;
            return false;
        }
        auto ifs = std::ifstream(this->walletPath, std::ios::binary);
        if(!ifs.is_open()) {
            std::cout << "Wallet File is not Exist" << std::endl;
            return false;
        }
        ifs.seekg(0,std::ios::end);
        const size_t fileSize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        std::cout << this->walletPath << ":" << fileSize << " [bytes]\n";
        return fileSize == 32;
    }

    void initWallet() {
        std::string address;
        if(this->isWalletExist()) {
            std::cout << "Load Private Key from " << this->walletPath << std::endl;
            auto ifs = std::ifstream(this->walletPath, std::ios::binary);
            auto privateKeyBytes = new char[32];
            ifs.read(privateKeyBytes, 32);
            this->privateKey.assign(privateKeyBytes, privateKeyBytes + 32);
            delete privateKeyBytes;
            ifs.close();
        } else {
            std::cout << "Generate Private Key" << std::endl;
            generateRandomBits(256, privateKey);

            auto privateKeyBytes = new char[32];
            std::copy(privateKey.begin(), privateKey.end(), privateKeyBytes);
            auto ofs = std::ofstream(this->walletPath, std::ios::binary);
            ofs.write(privateKeyBytes, 32);
            std::cout << "Save Private Key to " << this->walletPath << std::endl;
            std::copy(privateKeyBytes, privateKeyBytes + 32, this->privateKey.begin());
            delete privateKeyBytes;
            ofs.close();
        }

        convertPrivateKeyToPublicKey(this->privateKey, this->publicKey);
        this->myAddress = pubkeyToAddress(this->publicKey);

        std::vector<char> privateHex, publicHex;
        bytesToHexString(this->privateKey, privateHex);
        bytesToHexString(this->publicKey, publicHex);

        std::cout << "=== PRIVATE KEY ===" << std::endl;
        for(auto& c: privateHex) {
            std::cout << c;
        }
        std::cout << " : " << this->privateKey.size() << " [bytes]" << std::endl;

        std::cout << "=== PUBLIC KEY ===" << std::endl;
        for(auto& c: publicHex) {
            std::cout << c;
        }
        std::cout << " : " << this->publicKey.size() << " [bytes]" << std::endl;

        std::cout << "=== ADDRESS ===" << std::endl;
        std::cout << this->myAddress << std::endl;
    }

    void start() {
        this->initWallet();
        auto onConnect = [&](std::pair<std::string, uint16_t> addr) {
            this->onConnect(addr);
        };
        auto onError = [&](std::pair<std::string, uint16_t> addr, std::string error) {
            this->onServerError(addr, error);
        };
        auto onReceive = [&](std::pair<std::string, uint16_t> fromAddr, std::string data) {
            this->onReceive(fromAddr, data);
        };

        this->server = std::make_shared<Server>(this->ioService, port, HEADER_SIZE);
        this->server->startAccept(onConnect, onError, onReceive);
        this->ioService.poll();

        std::cout << "[p2p] listening on 9999" << std::endl;

        this->client = std::make_shared<Client>(this->ioService, 3, 100);

        if(this->peers.size() > 0) {
            auto lastId = this->activeChain[this->activeChain.size() - 1]->id();
            sendToPeer(std::dynamic_pointer_cast<AbstractMessage>(std::make_shared<GetBlocks>(lastId)));
            std::cout << "start initial block download from " << this->peers.size() << " peers in 60 secs" << std::endl;
        }
        while(true) {
            this->ioService.poll();
            sleep(1);
        }
    }

    void onConnect(std::pair<std::string, uint16_t> addr) {
        std::cout << "On connect to peer -> " << addr.first << ":" << addr.second << std::endl;
    }

    void onServerError(std::pair<std::string, uint16_t> addr, std::string error) {
        std::cout << "On server error: " << error;
    }

    void onReceive(std::pair<std::string, uint16_t> fromAddr, std::string data) {
        std::cout << "On data receive: " << data;
        auto json = nlohmann::json::parse(data);

        std::string type = json["type"];
        if(type == "GetBlocks") {
            return this->handleGetBlocksMessage(fromAddr, json);
        }
        if(type == "ResponseBlocks") {
            return this->handleResponseBlocks(fromAddr, json);
        }
        if(type == "PostTransaction") {
            return this->handlePostTransaction(fromAddr, json);
        }
        if(type == "PostBlock") {
            return this->handlePostBlock(fromAddr, json);
        }
        if(type == "GetUtxos") {
            return this->handleGetUtxos(fromAddr, json);
        }
        if(type == "ResponseUtxos") {
            return this->handleResponseUtxos(fromAddr, json);
        }
        if(type == "GetMempoolKeys") {
            return this->handleGetMempoolKeys(fromAddr, json);
        }
        if(type == "ResponseMempoolKeys") {
            return this->handleResponseMempoolKeys(fromAddr, json);
        }
        if(type == "GetActiveChain") {
            return this->handleGetActiveChain(fromAddr, json);
        }
        if(type == "ResponseActiveChain") {
            return this->handleResponseActiveChain(fromAddr, json);
        }
        if(type == "PosePeerInfo") {
            return this->handlePostPeerInfo(fromAddr, json);
        }
    }

    void handleGetBlocksMessage(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto getBlocks = GetBlocks(json);
        std::cout << "[p2p] receive GetBlocks from " << fromAddr.first << std::endl;
        auto locate = this->locateBlock(getBlocks.fromBlockId);
        const size_t chunckSize = 50;
        const size_t left  = (locate != nullptr ? locate->height : 1);
        const size_t right = std::min(this->activeChain.size(), left + chunckSize);

        BlockChain sendChain(right - left);
        this->chainLock.lock();
        std::copy(this->activeChain.begin() + left, this->activeChain.begin() + right, sendChain.begin());
        this->chainLock.unlock();

        std::cout << "[p2p] sending " << sendChain.size() << " blocks to " << fromAddr.first << std::endl;
        this->ioService.post(boost::bind(&TinyChain::sendToPeer, this, std::dynamic_pointer_cast<AbstractMessage>(std::make_shared<ResponseBlocks>(sendChain)), fromAddr.first, fromAddr.second));
    }

    void handleResponseBlocks(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto responseBlocks = ResponseBlocks(json);
        std::cout << "[p2p] receive ResponseBlocks from " << fromAddr.first << std::endl;

        int numNewConnect = 0;
        for(const auto& block: responseBlocks.blocks) {
            auto locate = this->locateBlock(block->id());
            if(locate == nullptr) {
                this->connectBlock(block);
                ++numNewConnect;
            }
        }

        if(numNewConnect == 0) {
            std::cout << "[p2p] initial block download complete" << std::endl;
            return;
        } else {
            const auto newTipId = this->activeChain[this->activeChain.size() - 1]->id();
            std::cout << "[p2p] continuing initial block download at " << newTipId << std::endl;
            return sendToPeer(std::dynamic_pointer_cast<AbstractMessage>(std::make_shared<GetBlocks>(newTipId)), fromAddr.first, fromAddr.second);
        }
    }

    void handlePostTransaction(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto postTransaction = PostTransaction(json);
        std::cout << "Received txn " << postTransaction.txn->id() << " from peer " << fromAddr.first << std::endl;

        try {
            postTransaction.txn->validate(this->utxoSet.utxoSet, this->mempool.mempool, this->activeChain.size());
            std::cout << "txn " << postTransaction.txn->id() << " added to mempool\n";
            this->mempool.mempool.insert(std::make_pair(postTransaction.txn->id(), postTransaction.txn));
        } catch(const Transaction::TransactionValidationException& e) {
            if(e.isOrphen) {
                this->orphanTxns.push_back(postTransaction.txn);
                std::cout << "txn " << postTransaction.txn->id() << " submitted as orphan\n";
            } else {
                std::cout << "txn rejected\n";
            }
        }

    }

    void handlePostBlock(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto postBlock = PostBlock(json);
        std::cout << "Received block " << postBlock.block->id() << " from peer " << fromAddr.first << std::endl;
        this->connectBlock(postBlock.block);
    }

    void handleGetUtxos(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto getUtxos = GetUtxos(json);
        auto responseUtxos = std::dynamic_pointer_cast<AbstractMessage>(std::make_shared<ResponseUtxos>(std::shared_ptr<UtxoSet>(&this->utxoSet)));
        return sendToPeer(responseUtxos, fromAddr.first, fromAddr.second);
    }

    void handleResponseUtxos(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto responseUtxos = ResponseUtxos(json);
    }

    void handleGetMempoolKeys(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto getMempoolKeys = GetMempoolKeys(json);
        auto responseMempoolKeys = std::dynamic_pointer_cast<AbstractMessage>(std::make_shared<ResponseMempoolKeys>(this->mempool.mempool));
        return sendToPeer(responseMempoolKeys, fromAddr.first, fromAddr.second);
    }

    void handleResponseMempoolKeys(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto responseMempoolKeys = ResponseMempoolKeys(json);
    }

    void handleGetActiveChain(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto getActiveChain = GetActiveChain(json);
        auto responseActiveChain = std::dynamic_pointer_cast<AbstractMessage>(std::make_shared<ResponseActiveChain>(this->activeChain));
    }

    void handleResponseActiveChain(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto responseActiveChain = ResponseActiveChain(json);
    }

    void handlePostPeerInfo(std::pair<std::string, uint16_t> fromAddr, nlohmann::json& json) {
        auto postPeerInfo = PostPeerInfo(json);
        this->peers.insert(fromAddr);
    }

    //client
    void sendToPeer(std::shared_ptr<AbstractMessage> message) {
        if(this->peers.size() == 0) {
            std::cout << "No peer" << std::endl;
            return;
        }
        size_t numPeers = this->peers.size();
        std::uniform_int_distribution<> range(0, numPeers - 1);
        int k = range(this->random);
        auto it = this->peers.begin();
        for(int i=0; i<k; ++i) {
            ++it;
        }

        this->sendToPeer(message, it->first, it->second);
    }

    void sendToPeer(std::shared_ptr<AbstractMessage> message, std::string peer, uint16_t port) {
        auto onFinish = [&](std::pair<std::string, uint16_t> addr, std::pair<enum Client::ResultCode, std::string> error) {
            this->onSendFinish(addr, error);
        };
        this->ioService.post([&]() {
            this->client->sendMessage(peer, port, message->toString(), onFinish);
        });
        this->ioService.poll();
    }

    void onSendFinish(std::pair<std::string, uint16_t> addr, std::pair<enum Client::ResultCode, std::string> error) {
        if(error.first == Client::ResultCode::CONNECT_FAILED) {
            this->peers.erase(addr);
        }
    }
};

#endif //TINYCHAIN_CPP_TINYCHAIN_H
