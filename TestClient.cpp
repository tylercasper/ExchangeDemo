#include "TestClient.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <sstream>
#include <fstream>

using boost::asio::ip::udp;
using namespace std;

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

        auto sender = [&](string& msg)
        {
            auto send_buf2 = boost::asio::buffer(msg);
            socket.send_to(boost::asio::buffer(send_buf2), receiver_endpoint);
            string str;
            boost::array<char, 128> recv_buf;
            while (str != "---eor")
            {
                recv_buf = boost::array<char, 128>();
                udp::endpoint sender_endpoint;
                size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

                str = string(recv_buf.begin(), recv_buf.begin() + len);

                if (str != "---eor")
                {
                    cout.write(recv_buf.data(), len);
                    cout << endl;
                }
                boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
            }
        };
//        auto str1 = string("N, 1, IBM, 10, 100, B, 1");
//        sender(str1);
//        auto str2 = string("N, 1, IBM, 9, 100, B, 2");
//        sender(str2);
//        auto str3 = string("N, 1, IBM, 11, 100, B, 3");
//        sender(str3);
//        auto str4 = string("N, 1, IBM, 11, 100, B, 4");
//        sender(str4);
//        auto str5 = string("N, 1, IBM, 11, 100, S, 5");
//        sender(str5);
//        auto str6 = string("C, 1, 3");
//        sender(str6);
//        auto str7 = string("C, 1, 1");
//        sender(str7);
//        auto str8 = string("C, 1, 4");
//        sender(str8);

        std::ifstream infile("../IO/inputFile.csv");
        std::string line;
        while (std::getline(infile, line))
        {
            if (line[0] != '#')
            {
                if(line.size() > 1)
                {
                    cout << "Input: " << line << endl;
                    sender(line);
                }
                else
                    cout << endl;
            }
            else
                cout << line;
        }

//        auto str1 = string("N, 1, IBM, 10, 100, B, 1");
//        sender(str1);
//        auto str2 = string("N, 1, IBM, 12, 100, S, 2");
//        sender(str2);
//        auto str3 = string("N, 2, IBM, 9, 100, B, 101");
//        sender(str3);
//        auto str4 = string("N, 2, IBM, 11, 100, S, 102");
//        sender(str4);
//        cout << endl;
//        auto str5 = string("N, 1, IBM, 11, 100, B, 3");
//        sender(str5);
//        auto str6 = string("N, 2, IBM, 10, 100, S, 103");
//        sender(str6);
//        cout << endl;
//        auto str7 = string("N, 1, IBM, 10, 100, B, 4");
//        sender(str7);
//        auto str8 = string("N, 2, IBM, 11, 100, S, 104");
//        sender(str8);
//        auto str9 = string("F");
//        sender(str9);


    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }
}