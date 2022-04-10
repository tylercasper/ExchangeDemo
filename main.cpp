#include "TradeEngine.h"
#include "TestClient.h"

int main()
{
    boost::asio::io_service io_service;
    auto server = make_shared<Server>(io_service);
    auto server_thread = boost::thread(boost::bind(&boost::asio::io_context::run, &io_service));

    TradeEngine trade_engine(server);
    auto engine_thread = boost::thread(boost::bind(&TradeEngine::run, &trade_engine));

    boost::thread client{runClient};

    client.join();
    /* Uncomment to keep server alive */
//    server_thread.join();
//    engine_thread.join();
    return 0;
}