//
// Created by kourin on 2018/08/20.
//

#ifndef TINYCHAIN_CPP_MESSAGE_H
#define TINYCHAIN_CPP_MESSAGE_H

#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <cassert>
#include <map>

#include "Block.h"
#include "Transaction.h"
#include "UtxoSet.h"
#include "UnspentTxOut.h"
#include "Mempool.h"

class AbstractMessage {
    public:
        virtual std::string toString() = 0;
};

class GetBlocks : public AbstractMessage {
    public:
        std::string fromBlockId;
        nlohmann::json json;

        GetBlocks(nlohmann::json& json)
            : json(json)
        {
            assert(json["type"] == "GetBlocks");
            this->fromBlockId = json["fromBlockId"];
        }

        GetBlocks(std::string fromBlockId)
            : json(nlohmann::json()), fromBlockId(fromBlockId)
        {
            this->json["type"] = "GetBlocks";
            this->json["fromBlockId"] = fromBlockId;
        }

        std::string toString() {
            return this->json.dump();
        }
};

class ResponseBlocks : public AbstractMessage {
    public:
        std::vector<std::shared_ptr<Block>> blocks;
        nlohmann::json json;

        ResponseBlocks(nlohmann::json& json)
            : json(json)
        {
            assert(json["type"] == "ResponseBlocks");
            size_t size = this->json["blocks"].size();
            this->blocks.resize(size);
            for(size_t i = 0; i < size; ++i) {
                this->blocks[i] = Block::deserialize(json["blocks"][i]);
            }
        }

        ResponseBlocks(std::vector<std::shared_ptr<Block>>& blocks)
            : blocks(blocks)
        {
            this->json["type"] = "ResponseBlocks";
            size_t size = blocks.size();
            for(size_t i = 0; i < size; ++i) {
                this->json["blocks"][i] = blocks[i]->serialize();
            }
        }

        std::string toString() {
            return json.dump();
        }
};

class PostTransaction : public AbstractMessage {
    public:
        std::shared_ptr<Transaction> txn;
        nlohmann::json json;
        PostTransaction(nlohmann::json& json)
        {
            assert(json["type"] == "PostTransaction");
            this->txn = Transaction::deserialize(json["transaction"]);
        }

        PostTransaction(std::shared_ptr<Transaction> txn) {
            this->json["type"] = "PostTransaction";
            this->json["transaction"] = txn->toString();
        }

        std::string toString() {
            return json.dump();
        }
};

class PostBlock : public AbstractMessage {
    public:
        std::shared_ptr<Block> block;
        nlohmann::json json;
        PostBlock(nlohmann::json& json)
            : json(json)
        {
            assert(json["type"] == "PostBlock");
            this->block = Block::deserialize(json["block"]);
        }

        PostBlock(std::shared_ptr<Block> block)
            : block(block)
        {
            this->json["type"] = "PostBlock";
            this->json["block"] = block->serialize();
        }

        std::string toString() {
            return json.dump();
        }
};

class GetUtxos : public AbstractMessage {
    public:
        nlohmann::json json;

        GetUtxos(nlohmann::json& json)
        : json(json)
        {
            assert(json["type"] == "GetUtxos");
        }

        GetUtxos()
        {
            this->json["type"] = "GetUtxos";
        }

        std::string toString() {
            return json.dump();
        }
};

class ResponseUtxos : public AbstractMessage {
    public:
        nlohmann::json json;
        std::shared_ptr<UtxoSet> utxoSet;
        ResponseUtxos(nlohmann::json& json)
            : json(json)
        {
            assert(json["type"] == "ResponseUtxos");
            this->utxoSet = std::make_shared<UtxoSet>();
            for(const auto& utxoJson: json["utxos"]) {
                auto utxo = UnspentTxOut::deserialize(utxoJson);
                this->utxoSet->utxoSet.insert(std::make_pair(utxo->outpoint(), *utxo));
            }
        }

        ResponseUtxos(std::shared_ptr<UtxoSet> utxoSet)
            : utxoSet(utxoSet)
        {
            this->json["type"] = "ResponseUtxos";
            int i = 0;
            for(auto kv : this->utxoSet->utxoSet) {
                this->json["utxos"][i++] = kv.second.toString();
            }
        }

        std::string toString() {
            return json.dump();
        }
};

class GetMempoolKeys : public AbstractMessage {
    public:
        nlohmann::json json;
        GetMempoolKeys(nlohmann::json& json)
            : json(json)
        {
            assert(json["type"] == "GetMempoolKeys");
        }

        GetMempoolKeys()
        {
            this->json["type"] = "GetMempoolKeys";
        }

        std::string toString() {
            return json.dump();
        }
};

class ResponseMempoolKeys : public AbstractMessage {
    public:
        nlohmann::json json;
        std::vector<std::string> keys;

        ResponseMempoolKeys(nlohmann::json& json)
            : json(json)
        {
            assert(json["type"] == "ResponseMempoolKeys");
            this->keys.reserve(json["keys"].size());
            for(auto& key: json["keys"]) {
                this->keys.push_back(key);
            }
        }

        ResponseMempoolKeys(std::map<std::string, std::shared_ptr<Transaction>>& mempool)
        {
            this->json["type"] = "ResponseMempoolKeys";
            for(auto& kv: mempool) {
                this->json["keys"].push_back(kv.first);
            }
        }

        std::string toString() {
            return json.dump();
        }
};

class GetActiveChain : public AbstractMessage {
    public:
        nlohmann::json json;

        GetActiveChain(nlohmann::json& json)
                : json(json)
        {
            assert(json["type"] == "GetActiveChain");
        }

        GetActiveChain()
        {
            json["type"] = "GetActiveChain";
        }

        std::string toString() {
            return json.dump();
        }
};

class ResponseActiveChain : public AbstractMessage {
    public:
        std::vector<std::shared_ptr<Block>> chain;
        nlohmann::json json;

        ResponseActiveChain(nlohmann::json& json)
                : json(json)
        {
            assert(json["type"] == "ResponseActiveChain");
            this->chain.reserve(json["blocks"].size());
            for(auto& blockJson: json["blocks"]) {
                this->chain.push_back(Block::deserialize(blockJson));
            }
        }

        ResponseActiveChain(std::vector<std::shared_ptr<Block>>& chain)
            : chain(chain)
        {
            json["type"] = "ResponseActiveChain";
            for(auto& block: chain) {
                json["blocks"].push_back(block->toString());
            }
        }

        std::string toString() {
            return json.dump();
        }
};

class PostPeerInfo : public AbstractMessage {
    public:
        std::string address;
        nlohmann::json json;


        PostPeerInfo(nlohmann::json& json)
                : json(json)
        {
            assert(json["type"] == "PosePeerInfo");
            this->address = json["address"];
        }

        PostPeerInfo(std::string& address)
            : address(address)
        {
            this->json["type"] = "PosePeerInfo";
            this->json["address"] = address;
        }


        std::string toString() {
            return json.dump();
        }
};



#endif //TINYCHAIN_CPP_MESSAGE_H
