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
                boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
            }
        };

        std::ifstream infile("../IO/inputFile.csv");
        std::string line;
        while (std::getline(infile, line))
        {
            if (line[0] != '#')
            {
                if (line.size() > 1 || line[0] == 'F')
                    sender(line);
                else
                    cout << endl;
            }
            else if (line.size() > 1 && line[1] == 's')
                cout << line;
        }
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }
}