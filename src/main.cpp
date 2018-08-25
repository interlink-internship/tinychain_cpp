#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <algorithm>
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
#include <memory>
#include <nlohmann/json.hpp>

#include "TinyChain.h"
#include "Client.h"
#include "Server.h"
#include "Connection.h"

#include "Message.h"

#include <stdio.h>
#include <secp256k1.h>
#include <stdio.h>

#include "Utility.h"

/*
void convertPrivateKeyToPublicKey(std::vector<unsigned char>& privateKeyBytes, std::vector<unsigned char>& publicKeyBytes) {
    publicKeyBytes.resize(64);
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_pubkey pubkey;
    secp256k1_ec_pubkey_create(ctx, &pubkey, privateKeyBytes.data());
    std::copy(pubkey.data, pubkey.data + 64, publicKeyBytes.begin());
}
*/

int main( int argc, char** argv) {
    assert(argc >= 2);

    /*
    std::vector<char> privateKeyHex, publicKeyHex;

    std::vector<unsigned char> privateKey;
    generateRandomBits(256, privateKey);
    auto privateKeyBytes = privateKey.data();

    std::cout << "=== Private Key ===" << std::endl;
    for(const auto c: privateKey) {
        std::cout << std::bitset<8>(c) << " ";
    }
    std::cout << std::endl;
    bytesToHexString(privateKey, privateKeyHex);
    for(const auto c: privateKeyHex) {
        std::cout << c;
    }
    std::cout << std::endl;

    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    secp256k1_pubkey pubkey;
    secp256k1_ec_pubkey_create(ctx, &pubkey, privateKeyBytes);
    std::cout << "=== Public Key ===" << std::endl;
    for(int i=0; i<64; ++i) {
        std::cout << std::bitset<8>(pubkey.data[i]) << " ";
    }
    std::cout << std::endl;
    std::vector<unsigned char> publicKey(pubkey.data, pubkey.data + 64);
    bytesToHexString(publicKey, publicKeyHex);
    for(const auto c: publicKeyHex) {
        std::cout << c;
    }
    std::cout << std::endl;

    std::vector<unsigned char> message(32, 0);
    std::vector<unsigned char> signature;
    generateSignature(privateKey, message, signature);

    std::vector<char> signatureHex;
    bytesToHexString(signature, signatureHex);
    std::cout << "=== SIGNATURE ===" << std::endl;
    for(const auto c: signatureHex) {
        std::cout << c;
    }
    std::cout << std::endl;

    message[0] = 1;

    std::cout << verifySignature(signature, message, publicKey) << std::endl;
*/
    /*
    auto msg = message.data();
    secp256k1_ecdsa_signature sig;
    secp256k1_pubkey pub;
    std::copy(signature.begin(), signature.end(), sig.data);
    auto x = secp256k1_ecdsa_verify(ctx, &sig, msg, &pubkey);
    std::cout << x << std::endl;
     */
    /*
    secp256k1_ecdsa_signature sig;
    secp256k1_ecdsa_sign(ctx, &sig, message, privateKeyBytes, NULL, NULL);
    std::vector<unsigned char> signature(sig.data, sig.data + 64);
    std::vector<char> signatureHex;
    bytesToHexString(signature, signatureHex);
    std::cout << "=== SIGNATURE ===" << std::endl;
    for(const auto c: signatureHex) {
        std::cout << c;
    }
    std::cout << std::endl;
     */

    std::set<std::pair<std::string, uint16_t>> hoge;
    hoge.insert(std::make_pair("127.0.0.1", 9999));
    if(argv[1][0] == '1') {
        boost::asio::io_service ioService;
        boost::asio::io_service::work work(ioService);
        auto tinychain = new TinyChain(ioService, "./chain.dat", "./wallet.dat", hoge, 9998);
        tinychain->start();
        sleep(10000);
    } else {
        boost::asio::io_service ioService;
        boost::asio::io_service::work work(ioService);
        auto tinychain = new TinyChain(ioService, "./chain.dat", "./wallet.dat", {}, 9999);
        tinychain->start();
        sleep(10000);
    }

/*
 *
 */

    /*
    std::string chainPath = "./chain.dat";

    TinyChain tinychain;
    tinychain.loadFromDisk(chainPath);
     */
    /*
    assert(argc >= 2);

    boost::asio::io_service ioService;
    boost::asio::io_service::work work(ioService);

    if(argv[1][0] == 's') {
        std::cout << "server mode" << std::endl;
        auto onConnect = std::bind([](std::pair<std::string, uint16_t> addr) {
            std::cout << "In main, onConnect -> " << addr.first << ":" << addr.second << std::endl;
        }, std::placeholders::_1);
        auto onError = std::bind([](std::string error) {
            std::cout << "Im main, onError -> " << error << std::endl;
        }, std::placeholders::_1);
        auto onReceive = std::bind([](std::string message) {
            std::cout << "Im main, onReceive -> " << message << std::endl;
        }, std::placeholders::_1);

        Server server(ioService, 9999);
        server.startAccept(onConnect, onError, onReceive);

        ioService.run();
        sleep(10000);
    } else {
        Client client(ioService, "127.0.0.1", 9999, 3, 100);
        auto onFinish = std::bind([](std::pair<enum Client::ResultCode, std::string> error){
            std::cout << "Im main, " << error.first << ":" << error.second << std::endl;
            exit(0);
        }, std::placeholders::_1);

        std::string message("hello, world");
        client.sendMessage(message, onFinish);

        ioService.run();
        sleep(10000);
    }
    return 0;
    */
}