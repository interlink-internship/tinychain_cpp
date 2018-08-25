//
// Created by kourin on 2018/08/20.
//

#ifndef TINYCHAIN_CPP_CLIENT_H
#define TINYCHAIN_CPP_CLIENT_H

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

class Client  {
public:
    Client(boost::asio::io_service& ioService, const unsigned int retry, const unsigned int timeout)
            : ioService(ioService), socket(ioService), timer(ioService), retry(retry), timeout(timeout)
    {};

    enum ResultCode {
        SUCCESS,
        SEND_FAILED,
        CONNECT_FAILED,
        TIMEOUT
    };

    void sendMessage(const std::string toAddr, const uint16_t port, const std::string message, std::function<void(std::pair<std::string, uint16_t>, std::pair<enum ResultCode, std::string>)> onFinish) {
        std::cout << "send message (" << toAddr << ":" << port << "): " << message << " " << message.size() << "[bytes]" << std::endl;
        this->setTimeout(toAddr, port, onFinish);
        boost::system::error_code ec;
        boost::asio::spawn(ioService, [&, message, onFinish](boost::asio::yield_context yield) {
            auto connectError = this->connect(toAddr, port, yield);
            if(connectError) {
                const std::string message = "Connection Failed, error code=" + connectError.message();
                this->resetTimeout();
                this->ioService.post([&](){
                    onFinish(std::make_pair(toAddr, port), std::make_pair(CONNECT_FAILED, message));
                });
                this->ioService.poll();
                return;
            }
            std::cout << "Connection Success" << std::endl;

            size_t size = message.size();
            auto sendHeaderError = this->sendHeader(size, 4, yield);
            if(sendHeaderError) {
                const std::string message = "Send Header Failed, error code=" + sendHeaderError.message();
                this->resetTimeout();
                this->ioService.post([&](){
                    onFinish(std::make_pair(toAddr, port), std::make_pair(SEND_FAILED, message));
                });
                this->ioService.poll();
                return;
            }
            std::cout << "Header Send Success" << std::endl;

            auto sendBodyError = this->sendBody(message, size, yield);
            if(sendBodyError) {
                const std::string message = "Send Failed, error code=" + sendBodyError.message();
                this->resetTimeout();
                this->ioService.post([&](){
                    onFinish(std::make_pair(toAddr, port), std::make_pair(SEND_FAILED, message));
                });
                this->ioService.poll();
                return;
            }
            std::cout << "Message Send Success" << std::endl;

            this->socket.close();
            this->resetTimeout();
            this->ioService.post([&](){
                onFinish(std::make_pair(toAddr, port), std::make_pair(SUCCESS, ""));
            });
            this->ioService.poll();
            return;
        });
    }

private:
    void setTimeout(const std::string& toAddr, const uint16_t port, std::function<void(std::pair<std::string, uint16_t>, std::pair<enum ResultCode, std::string>)> onFinish) {
        std::cout << "on setTimeout: " << this->timeout << std::endl;
        if(this->timeout > 0) {
            std::cout << "set timeout" << std::endl;
            auto duration = boost::posix_time::seconds(this->timeout);
            this->timer.expires_from_now(duration);
            this->timer.async_wait(std::bind(&Client::onTimeout, this, std::placeholders::_1, toAddr, port, onFinish));
        }
    }

    void resetTimeout() {
        this->timer.cancel();
    }

    void onTimeout(const boost::system::error_code& ec, const std::string toAddr, const uint16_t port, std::function<void(std::pair<std::string, uint16_t>, std::pair<enum ResultCode, std::string>)> onFinish) {
        if (this->timer.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
            std::cout << "timeout" << std::endl;
            this->socket.close();
            this->ioService.post([&](){
                onFinish(std::make_pair(toAddr, port), std::make_pair(TIMEOUT, ""));
            });
            this->ioService.run();
        }
    }

    boost::system::error_code connect(const std::string toAddr, const uint16_t port, boost::asio::yield_context yield) {
        std::cout << "Connect to " << toAddr << ":" << port << "\n";

        boost::system::error_code ec;
        for(unsigned int i=0; i<=this->retry; ++i) {
            auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(toAddr), port);
            this->socket.async_connect(endpoint, yield[ec]);
            if(!ec) break;
            std::cout << "connect failed" << std::endl;
            sleep(1);
        }
        return ec;
    }

    boost::system::error_code sendHeader(size_t value, size_t length, boost::asio::yield_context yield) {
        std::vector<unsigned char> bytes;
        integerToBytes(value, length, bytes);

        auto buffer = boost::asio::buffer(bytes.data(), length);
        std::cout << "Send Header Size:" << buffer.size() << std::endl;
        boost::system::error_code ec;
        boost::asio::async_write(this->socket, buffer, yield[ec]);
        return ec;
    }


    boost::system::error_code sendBody(const std::string& message, const size_t& size, boost::asio::yield_context yield) {
        boost::system::error_code ec;
        auto buffer = boost::asio::buffer(message, message.length());
        std::cout << message << ":" << boost::asio::buffer_size(buffer) << std::endl;
        boost::asio::async_write(this->socket, boost::asio::buffer(message), yield[ec]);
        return ec;
    }

    boost::asio::io_service& ioService;
    boost::asio::ip::tcp::socket socket;

    boost::asio::deadline_timer timer;

    const unsigned int retry;
    const unsigned int timeout;
};

#endif //TINYCHAIN_CPP_CLIENT_H
