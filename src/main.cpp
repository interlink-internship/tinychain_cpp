#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>
#include <iostream>
#include <string>

#include "Transaction.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

#include "sha256.h"

int main(int argc, char* argv[]) {
    //std::cout << h1 << std::endl;
    auto txo = Transaction();

    std::cout << "serialize:\n" << txo.serialize()<< std::endl;
    std::cout << "double sha256 of serialize:\n" << sha256(sha256(txo.serialize())) << std::endl;
    std::cout << "id:\n" << txo.id() << std::endl;

    std::cout << 0 << std::endl;
}