//
// Created by kourin on 2018/08/20.
//

#ifndef TINYCHAIN_CPP_CONNECTION_H
#define TINYCHAIN_CPP_CONNECTION_H

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

#include "Utility.h"

class Connection : public boost::enable_shared_from_this<Connection> {
public:
    const size_t headerSize;

    typedef boost::shared_ptr<Connection> Pointer;
    boost::asio::io_service& ioService;
    boost::asio::ip::tcp::socket socket;
    boost::asio::streambuf streamBuffer;

    std::function<void(std::pair<std::string, uint16_t>)> onConnect;
    std::function<void(std::pair<std::string, uint16_t>, std::string)> onError;
    std::function<void(std::pair<std::string, uint16_t>, std::string)> onReceive;

    static Pointer create(boost::asio::io_service& ioService, const size_t headerSize, std::function<void(std::pair<std::string, uint16_t>)>& onConnect, std::function<void(std::pair<std::string, uint16_t>, std::string)>& onError, std::function<void(std::pair<std::string, uint16_t>, std::string)>& onReceive) {
        return Pointer(new Connection(ioService, headerSize, onConnect, onError, onReceive));
    }

    void start() {
        auto address = this->socket.remote_endpoint().address().to_string();
        auto port = this->socket.remote_endpoint().port();
        std::cout << "New Connection from " << this->socket.remote_endpoint().address() << ":" << this->socket.remote_endpoint().port() << "\n";
        this->ioService.post(boost::bind(this->onConnect, std::make_pair(address, port)));
        this->ioService.poll();
        boost::asio::async_read(this->socket, this->streamBuffer, boost::asio::transfer_at_least(this->headerSize), boost::bind(&Connection::onReceiveHeader, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

private:
    Connection(boost::asio::io_service& ioService, const size_t headerSize, std::function<void(std::pair<std::string, uint16_t>)> onConnect, std::function<void(std::pair<std::string, uint16_t>, std::string)> onError, std::function<void(std::pair<std::string, uint16_t>, std::string)> onReceive)
            : ioService(ioService), socket(ioService), headerSize(headerSize), onConnect(onConnect), onError(onError), onReceive(onReceive)
    {
        std::cout << "new connection create!\n";
    }

    void onReceiveHeader(const boost::system::error_code& ec, size_t bytes_transferred) {
        auto address = this->socket.remote_endpoint().address().to_string();
        auto port = this->socket.remote_endpoint().port();

        if(ec && ec != boost::asio::error::eof) {
            std::string message = "Receive Header Error: " + ec.message();
            std::cout << message << std::endl;
            this->socket.close();
            this->ioService.post(boost::bind(this->onError, std::make_pair(address, port), message));
            return;
        }

        auto bytes = boost::asio::buffer_cast<const unsigned char*>(this->streamBuffer.data());
        streamBuffer.consume(this->headerSize);
        const auto dataSize = bytesToInteger<size_t>(headerSize, bytes);
        std::cout << "Header: " << dataSize << "[bytes]" << std::endl;
        boost::asio::async_read(this->socket, this->streamBuffer, boost::asio::transfer_at_least(dataSize), boost::bind(&Connection::onReceiveBody, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, dataSize));
    }

    void onReceiveBody(const boost::system::error_code& ec, size_t bytes_transferred, size_t size) {
        auto address = this->socket.remote_endpoint().address().to_string();
        auto port = this->socket.remote_endpoint().port();

        if(ec && ec != boost::asio::error::eof) {
            std::string message = "Receive Body Error: " + ec.message();
            std::cout << message << std::endl;
            this->socket.close();
            this->ioService.post(boost::bind(this->onError, std::make_pair(address, port), message));
            return;
        }

        auto bytes = boost::asio::buffer_cast<const unsigned char*>(this->streamBuffer.data());
        this->streamBuffer.consume(size);
        this->socket.close();
        std::cout << "Body:" << std::string(bytes, bytes + size) << std::endl;
        //this->onReceive(std::string(bytes, bytes + size));
        this->ioService.post(boost::bind(this->onReceive, std::make_pair(address, port), std::string(bytes, bytes + size)));
    }
};

#endif //TINYCHAIN_CPP_CONNECTION_H
