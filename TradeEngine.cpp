#include "TradeEngine.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/regex.hpp>

TradeEngine::TradeEngine(std::shared_ptr<Server> server):
server(server)
{}

void TradeEngine::run()
{
    std::string input;
    while(true)
    {
        if (!server->isInputQueueEmpty())
            server->setProcessing(true);
        input = server->processInputQueue();
        while (!input.empty())
        {
            auto temp = input;
//            std::cout << "Engine: processing string: " << input << std::endl;
            std::string delimiter = ", ";
            std::string token = input.substr(0, input.find(delimiter));
            input.erase(0, input.find(delimiter) + delimiter.length());
            if (token == std::string("N"))
            {
                processNewOrder(input);
            } else if (token == std::string("C"))
            {
                processCancelOrder(input);
            } else if (token == std::string("F"))
            {
                orderBook.clear();
            }
//            std::cout << "Engine: finished processing order" << std::endl;
            server->pushToOutputQueue("---eof");
            input = server->processInputQueue();
        }
        server->setProcessing(false);
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
}

void TradeEngine::processNewOrder(std::string& order)
{
//    std::cout << "Engine: processing new order string: " << order << std::endl;
    std::vector<std::string> tokenized_order;
    boost::algorithm::split_regex(tokenized_order, order, boost::regex(", "));
    auto ord = Order(tokenized_order);
//    std::cout << "Engine: processing new order: " << ord.tobOrdToString() << std::endl;

    ord.time_placed = time(nullptr);
    server->pushToOutputQueue(std::string("A, " + std::to_string(ord.user) + ", " + std::to_string(ord.user_order_id)));

    auto ord_match = orderBook.getTopMatchingOrder(ord);
    if (ord_match == nullptr && ord.price != 0)
    {
        addToBook(ord);
    }
    else
    {
        takeOrder(ord, ord_match);

        if (ord.quantity > 0 && ord.price != 0)
            addToBook(ord);
    }

}

void TradeEngine::processCancelOrder(std::string& order)
{
//    std::cout << "Engine: canceling order: " << order << std::endl;
    std::string delimiter = ", ";
    std::string user = order.substr(0, order.find(delimiter));
    order.erase(0, order.find(delimiter) + delimiter.length());
    std::string id = order.substr(0, order.find(delimiter));
    auto top = orderBook.removeOrderFromBook(user, id);
    server->pushToOutputQueue(std::string("A, " + user + ", " + id));
    if (!top.empty())
        server->pushToOutputQueue(top);

}

void TradeEngine::takeOrder(Order &order, std::shared_ptr<Order> match)
{
    using namespace std;
    std::shared_ptr<Order> buy;
    std::shared_ptr<Order> sell;
    if (order.side == 'B')
    {
        buy = make_shared<Order>(order);
        sell = match;
    }
    else
    {
        buy = match;
        sell = make_shared<Order>(order);
    }
    while(order.quantity > 0 && match != nullptr)
    {
        int diff = 0;
        if (order.quantity >= match->quantity)
        {
            diff = match->quantity;
            order.quantity -= diff;
            auto top = orderBook.removeOrderFromBook(match->order_id);
            if (!top.empty())
                server->pushToOutputQueue(top);
        }
        else
        {
            diff = order.quantity;
            match->quantity -= diff;
            order.quantity = 0;
            orderBook.removeOrderFromBook(match->order_id);
            auto top = orderBook.addOrderToBook(*match);
            if (!top.empty())
                server->pushToOutputQueue(top);
        }

        server->pushToOutputQueue(string("T, " + to_string(buy->user) + ", " + to_string(buy->user_order_id) + ", " +
                                                 to_string(sell->user) + ", " + to_string(sell->user_order_id) + ", " +
                                                 to_string(diff) + ", " + to_string(order.price)));
        match = orderBook.getTopMatchingOrder(order);
    }
}

void TradeEngine::addToBook(Order &order)
{
    auto top = orderBook.addOrderToBook(order);
    if (!top.empty())
    {
        server->pushToOutputQueue(top);
    }
}
