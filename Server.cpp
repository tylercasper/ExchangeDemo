#include "Server.h"
#include <boost/asio/detached.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>

using namespace boost::system;
using namespace boost::asio;
using boost::asio::ip::udp;

Server::Server(io_service& io_service)
: socket(io_service, udp::endpoint(udp::v4(), 15))
{
    std::cout << "Server Running" << std::endl;
    start_receive();
}

std::string Server::processInputQueue()
{
    if (inputBuffer.empty())
        return "";

    return inputBuffer.pop_front();
}

void Server::start_receive()
{
//    std::cout << "Start receive" << std::endl;
    socket.async_receive_from(buffer(received_buff), dest,
                              boost::bind(&Server::handle_receive, this, placeholders::error,
                                          placeholders::bytes_transferred));
}

void Server::handle_receive(const error_code& error, std::size_t bytes_transferred)
{
    auto str = std::string(received_buff.begin(), received_buff.begin() + bytes_transferred);
//    std::cout << "Handle receive: " << str << std::endl;
    inputBuffer.push(str);
    if (!error || error == error::message_size)
    {
        std::shared_ptr<std::string> message;
        while (!outputBuffer.empty() || !inputBuffer.empty() || buffer_processing)
        {
//            std::cout << "here1" << std::endl;
            if (!outputBuffer.empty())
            {
                message = std::make_shared<std::string>(std::string(outputBuffer.pop_front()));
//                std::cout << "here2" << std::endl;
                socket.async_send_to(buffer(*message), dest,
                                     boost::bind(&Server::handle_send, this, message,
                                                 placeholders::error, placeholders::bytes_transferred));

//                std::cout << "here3" << std::endl;
                start_receive();
            }
            boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
        }
//        std::cout << "Server: processed request: " << str << std::endl;
//        message = std::make_shared<std::string>("\n");
//        socket.async_send_to(buffer(*message), dest,
//                             boost::bind(&Server::handle_send, this, message,
//                                         placeholders::error, placeholders::bytes_transferred));

        start_receive();
    }
}

void Server::handle_send(std::shared_ptr<std::string>& str, const error_code&, std::size_t)
{

}

void Server::pushToOutputQueue(const std::string& str)
{
//    std::cout << "Server: pushing to output buffer: " << str << std::endl;
    outputBuffer.push(str);
}

void Server::setProcessing(bool p)
{
    buffer_processing = p;
}

bool Server::isInputQueueEmpty()
{
    return inputBuffer.empty();
}


std::string make_output()
{
    std::time_t now = time(nullptr);
    return std::ctime(&now);
}