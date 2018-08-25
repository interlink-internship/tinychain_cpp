//
// Created by kourin on 2018/08/20.
//

#ifndef TINYCHAIN_CPP_SERVER_H
#define TINYCHAIN_CPP_SERVER_H

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

#include "Connection.h"

class Server {
    boost::asio::io_service& ioService;
    boost::asio::ip::tcp::acceptor acceptor;
    size_t headerSize;

    public:
        Server(boost::asio::io_service& ioService, uint16_t port, size_t headerSize)
            : ioService(ioService), acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), headerSize(headerSize)
        {}

        void startAccept(std::function<void(std::pair<std::string, uint16_t>)> onConnect, std::function<void(std::pair<std::string, uint16_t>, std::string)> onError, std::function<void(std::pair<std::string, uint16_t>, std::string)> onReceive) {
            std::cout << "start accept" << std::endl;

            Connection::Pointer newConnection = Connection::create(this->acceptor.get_io_service(), this->headerSize, onConnect, onError, onReceive);
            this->acceptor.async_accept(newConnection->socket, boost::bind(&Server::onAccept, this, newConnection, boost::asio::placeholders::error, onConnect, onError, onReceive));
        }

        void onAccept(Connection::Pointer newConnection, const boost::system::error_code& error, std::function<void(std::pair<std::string, uint16_t>)>& onConnect, std::function<void(std::pair<std::string, uint16_t>, std::string)>& onError, std::function<void(std::pair<std::string, uint16_t>, std::string)>& onReceive) {
            if(!error) {
                std::cout << "New Client Connected" << std::endl;
                newConnection->start();
            }
            this->startAccept(onConnect, onError, onReceive);
        }
};

#endif //TINYCHAIN_CPP_SERVER_H
