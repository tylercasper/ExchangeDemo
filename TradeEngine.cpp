#include "TradeEngine.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/regex.hpp>

using namespace std;

TradeEngine::TradeEngine(shared_ptr<Server> server):
server(server)
{}

void TradeEngine::run()
{
    string input;
    try
    {
        while (true)
        {
            if (!server->isInputQueueEmpty())
                server->setProcessing(true);
            input = server->processInputQueue();
            while (!input.empty())
            {
                string delimiter = ", ";
                string token = input.substr(0, input.find(delimiter));
                input.erase(0, input.find(delimiter) + delimiter.length());
                if (token == "N")
                {
                    processNewOrder(input);
                } else if (token == "C")
                {
                    processCancelOrder(input);
                } else if (token[0] == 'F')
                {
                    orderBook.clear();
                }
                server->pushToOutputQueue("---eor");
                input = server->processInputQueue();
            }
            server->setProcessing(false);
            boost::this_thread::sleep_for(boost::chrono::milliseconds(2));
        }
    } catch (boost::thread_interrupted&) {
    }
}

void TradeEngine::processNewOrder(string& order)
{
    vector<string> tokenized_order;
    boost::algorithm::split_regex(tokenized_order, order, boost::regex(", "));
    auto ord = Order(tokenized_order);
    ord.timePlaced = clock();
    server->pushToOutputQueue(string("A, " + to_string(ord.user) + ", " + to_string(ord.userOrderId)));

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

void TradeEngine::processCancelOrder(string& order)
{
    string delimiter = ", ";
    string user = order.substr(0, order.find(delimiter));
    order.erase(0, order.find(delimiter) + delimiter.length());
    string id = order.substr(0, order.find(delimiter));

    string top = orderBook.removeOrderFromBook(user, id);
    server->pushToOutputQueue(string("A, " + user + ", " + id));
    if (!top.empty())
        server->pushToOutputQueue(top);
}

void TradeEngine::takeOrder(Order &order, shared_ptr<Order> match)
{
    shared_ptr<shared_ptr<Order>> buy = make_shared<shared_ptr<Order>>(match);
    shared_ptr<shared_ptr<Order>> sell = make_shared<shared_ptr<Order>>(match);
    if (order.side == 'B')
        buy = make_shared<shared_ptr<Order>>(make_shared<Order>(order));
    else
        sell = make_shared<shared_ptr<Order>>(make_shared<Order>(order));
    auto& tmp_srv = server;
    auto printr = [&buy, &sell, &tmp_srv, &order](const int diff, const shared_ptr<Order>& match)
    {
        tmp_srv->pushToOutputQueue(string("T, " + to_string((*buy)->user) + ", " + to_string((*buy)->userOrderId) + ", " +
                                       to_string((*sell)->user) + ", " + to_string((*sell)->userOrderId) + ", " +
                                       to_string(match->price) + ", " + to_string(diff)));
    };
    while(order.quantity > 0 && match != nullptr)
    {
        int diff = 0;

        if (order.quantity >= match->quantity)
        {
            diff = match->quantity;
            order.quantity -= diff;
            printr(diff, match);
            auto top = orderBook.removeOrderFromBook(match->orderId);
            if (!top.empty())
                server->pushToOutputQueue(top);
        }
        else
        {
            diff = order.quantity;
            match->quantity -= diff;
            order.quantity = 0;
            printr(diff, match);
            orderBook.removeOrderFromBook(match->orderId);
            auto top = orderBook.addOrderToBook(*match);
            if (!top.empty())
                server->pushToOutputQueue(top);
        }

        match = orderBook.getTopMatchingOrder(order);
    }
}

void TradeEngine::addToBook(Order &order)
{
    auto top = orderBook.addOrderToBook(order);
    if (!top.empty())
        server->pushToOutputQueue(top);
}
