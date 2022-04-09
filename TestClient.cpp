#include "TestClient.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
using boost::asio::ip::udp;

void runClient()
{
    try
    {
        boost::asio::io_service io_service;

        udp::resolver resolver(io_service);
        udp::resolver::query query(udp::v4(), "localhost", "15");
        udp::endpoint receiver_endpoint = *resolver.resolve(query);

        udp::socket socket(io_service);
        socket.open(udp::v4());

        auto sender = [&](std::string& msg)
        {
            auto send_buf2 = boost::asio::buffer(msg);
//        boost::array<char, 1> send_buf  = { 'a' };
//            std::cout << "Client: sending message: " << msg << std::endl;
            socket.send_to(boost::asio::buffer(send_buf2), receiver_endpoint);
//            std::cout << "Client: message sent." << std::endl;
            std::string str;
            boost::array<char, 128> recv_buf;
            while (str != "---eof")
            {
                recv_buf = boost::array<char, 128>();
                udp::endpoint sender_endpoint;
//                std::cout << "Client: waiting for response" << std::endl;
                size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

                str = std::string(recv_buf.begin(), recv_buf.begin() + len);

                if (str == "---eof")
                {
//                    std::cout << "Client: end request: " << str << std::endl << std::endl;
                }
                else
                {
//                    std::cout << "Client: response received: ";
                    std::cout.write(recv_buf.data(), len);
                    std::cout << std::endl;
                }
                boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
            }
        };
//        auto str1 = std::string("N, 1, IBM, 10, 100, B, 1");
//        sender(str1);
//        auto str2 = std::string("N, 1, IBM, 9, 100, B, 2");
//        sender(str2);
//        auto str3 = std::string("N, 1, IBM, 11, 100, B, 3");
//        sender(str3);
//        auto str4 = std::string("N, 1, IBM, 11, 100, B, 4");
//        sender(str4);
//        auto str5 = std::string("N, 1, IBM, 11, 100, S, 5");
//        sender(str5);
//        auto str6 = std::string("C, 1, 3");
//        sender(str6);
//        auto str7 = std::string("C, 1, 1");
//        sender(str7);
//        auto str8 = std::string("C, 1, 4");
//        sender(str8);

        auto str1 = std::string("N, 1, IBM, 10, 100, B, 1");
        sender(str1);
        auto str2 = std::string("N, 1, IBM, 12, 100, S, 2");
        sender(str2);
        auto str3 = std::string("N, 2, IBM, 9, 100, B, 101");
        sender(str3);
        auto str4 = std::string("N, 2, IBM, 11, 100, S, 102");
        sender(str4);
        std::cout << std::endl;
        auto str5 = std::string("N, 1, IBM, 11, 100, B, 3");
        sender(str5);
        auto str6 = std::string("N, 2, IBM, 10, 100, S, 103");
        sender(str6);
        std::cout << std::endl;
        auto str7 = std::string("N, 1, IBM, 10, 100, B, 4");
        sender(str7);
        auto str8 = std::string("N, 2, IBM, 11, 100, S, 104");
        sender(str8);
        auto str9 = std::string("F");
        sender(str9);


    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}