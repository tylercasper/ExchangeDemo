#include "Server.h"
#include <boost/asio/detached.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>

using namespace boost::asio::placeholders;
using namespace std;
using boost::asio::ip::udp;

Server::Server(boost::asio::io_service& io_service)
: socket(io_service, udp::endpoint(udp::v4(), 15))
{
    start_receive();
}

string Server::processInputQueue()
{
    if (inputBuffer.empty())
        return "";

    return inputBuffer.pop_front();
}

void Server::start_receive()
{
    socket.async_receive_from(boost::asio::buffer(received_buff), dest,
                              boost::bind(&Server::handle_receive,this, error,bytes_transferred));
}

void Server::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
    auto str = string(received_buff.begin(), received_buff.begin() + bytes_transferred);
    inputBuffer.push(str);
    if (!error)
    {
        shared_ptr<string> message;
        while (!outputBuffer.empty() || !inputBuffer.empty() || buffer_processing)
        {
            if (!outputBuffer.empty())
            {
                message = make_shared<string>(string(outputBuffer.pop_front()));
                socket.async_send_to(boost::asio::buffer(*message), dest,
                                     boost::bind(&Server::handle_send, this, message,error,
                                                 bytes_transferred));

                start_receive();
            }
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
        }

        start_receive();
    }
}

void Server::handle_send(shared_ptr<string>& str, const boost::system::error_code&, size_t)
{}

void Server::pushToOutputQueue(const string& str)
{
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