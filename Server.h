#include "TradeEngineConfig.h"
#include "Types.h"
#include <iostream>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#ifndef TRADEENGINE_SERVER_H
#define TRADEENGINE_SERVER_H

class Server
{
public:
    Server(boost::asio::io_service& io_service);
    std::string processInputQueue();
    bool isInputQueueEmpty();
    void pushToOutputQueue(const std::string& str);
    void setProcessing(bool p);

private:
    void start_receive();
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_send(std::shared_ptr<std::string>&, const boost::system::error_code&, std::size_t);

    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint dest;
    boost::array<char, 1024> received_buff;
    ThreadSafeQueue inputBuffer;
    ThreadSafeQueue outputBuffer;
    bool buffer_processing{false};
};

std::string make_output();
void runServer();

#endif //TRADEENGINE_SERVER_H
