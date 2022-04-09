#include "TradeEngine.h"
#include "TestClient.h"

int main()
{
    boost::asio::io_service io_service;
    auto server = make_shared<Server>(io_service);
    auto server_thread = boost::thread(boost::bind(&boost::asio::io_context::run, &io_service));
    std::cout << "Main thread: server running." << std::endl;

    TradeEngine trade_engine(server);
    auto engine_thread = boost::thread(boost::bind(&TradeEngine::run, &trade_engine));
    std::cout << "Main thread: engine running." << std::endl;

    boost::thread client{runClient};
    std::cout << "Main thread: client running." << std::endl;

    client.join();
    server_thread.join();
    engine_thread.join();
    return 0;
}