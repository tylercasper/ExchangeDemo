#include "Types.h"
#include <iostream>
using std::string;

void ThreadSafeQueue::push(const string& str)
{
    boost::mutex::scoped_lock lock(mutex);
    queue.push(str);
}

bool ThreadSafeQueue::empty()
{
    boost::mutex::scoped_lock lock(mutex);
    return queue.empty();
}

string ThreadSafeQueue::pop_front()
{
    boost::mutex::scoped_lock lock(mutex);
    auto str = queue.front();
    queue.pop();
    return str;
}

Order::Order(int user, const string& symbol, int price, int quantity, char side, int user_ord_id):
        user(user), symbol(symbol), price(price), quantity(quantity), side(side), userOrderId(user_ord_id),
        orderId(string(std::to_string(user) + "." + std::to_string(user_ord_id)))
{}

Order::Order(const std::vector<string>& ord_str):
    Order(stoi(ord_str[0]), ord_str[1], stoi(ord_str[2]), stoi(ord_str[3]),
          (char)ord_str[4][0], stoi(ord_str[5]))
{}

string Order::tobOrdToString() const
{
    return string("B, " + string(1, side) + ", " + std::to_string(price) + ", " + std::to_string(quantity));
}

std::shared_ptr<Order> OrderBook::getOrder(const string& order_id)
{
    string book = orderBookLookup[order_id];
    auto it = orderBookSell[book].find(order_id);
    if (it != orderBookSell[book].end())
        return std::make_shared<Order>(it->second);
    else if (it = orderBookBuy[book].find(order_id); it != orderBookBuy[book].end())
        return std::make_shared<Order>(it->second);

    return nullptr;
}

std::shared_ptr<Order> OrderBook::getTopMatchingOrder(const Order& ord)
{
    if (ord.side == 'B') {
        auto it = orderBookSellInverse[ord.symbol].begin();
        if (it != orderBookSellInverse[ord.symbol].end() && (it->first.price <= ord.price || ord.price == 0))
            return std::make_shared<Order>(it->first);
    }
    else
    {
        auto it = orderBookBuyInverse[ord.symbol].begin();
        if (it != orderBookBuyInverse[ord.symbol].end() && (it->first.price >= ord.price || ord.price == 0))
            return std::make_shared<Order>(it->first);
    }
    return nullptr;
}

string OrderBook::addOrderToBook(const Order& ord)
{
    string top;
    orderBookLookup[ord.orderId] = ord.symbol;

    auto inserter = [&ord, &top](auto& map, auto& map_inverse)
    {
        map[ord.symbol].insert({ord.orderId, ord});
        map_inverse[ord.symbol].insert({ord, ord.orderId});
        if (ord.price == map_inverse[ord.symbol].begin()->first.price)
        {
            auto it = map_inverse[ord.symbol].begin();
            auto tmp_ord = it->first;
            for (++it; it->first.price == tmp_ord.price; it++)
                tmp_ord.quantity += it->first.quantity;
            top = tmp_ord.tobOrdToString();
        }
    };
    if (!orderBookBuy.contains(ord.symbol))
        initMaps(ord);

    if (ord.side == 'B')
        inserter(orderBookBuy, orderBookBuyInverse);
    else
        inserter(orderBookSell, orderBookSellInverse);
    return top;
}

string OrderBook::removeOrderFromBook(const string& order_id)
{
    string top;
    string book = orderBookLookup[order_id];


    auto remover = [&book, &top, &order_id](auto& map, auto& map_inverse)
    {
        auto it = map[book].find(order_id);
        if (it->second.orderId == map_inverse[book].begin()->second)
        {
            if (map_inverse[book].size() == 1)
                top = string("B, " + string(1, it->second.side) + ", -, -");
            else
                top = (++map_inverse[book].begin())->first.tobOrdToString();
        }

        map_inverse[book].erase(it->second);
        map[book].erase(order_id);
    };

    if (orderBookSell[book].contains(order_id))
        remover(orderBookSell, orderBookSellInverse);
    else if (orderBookBuy[book].contains(order_id))
        remover(orderBookBuy, orderBookBuyInverse);

    return top;
}

void OrderBook::clear()
{
    for (auto& [key, value]: orderBookBuy)
    {
        value.clear();
    }
    orderBookBuy.clear();
    for (auto &[key, value]: orderBookBuyInverse)
    {
        value.clear();
    }
    orderBookBuyInverse.clear();
    for (auto &[key, value]: orderBookSell)
    {
        value.clear();
    }
    orderBookSell.clear();
    for (auto &[key, value]: orderBookSellInverse)
    {
        value.clear();
    }
    orderBookSellInverse.clear();
    orderBookLookup.clear();
}

string OrderBook::removeOrderFromBook(const string& user_id, const string& user_order_id)
{
    return removeOrderFromBook(user_id + "." + user_order_id);
}

void OrderBook::initMaps(const Order& ord)
{
        orderBookBuy.insert({ord.symbol, std::map<string, Order>()});
        orderBookBuyInverse.insert({ord.symbol, std::map<Order, string, OrdCompBuy>()});
        orderBookSell.insert({ord.symbol, std::map<string, Order>()});
        orderBookSellInverse.insert({ord.symbol, std::map<Order, string, OrdCompSell>()});
}

bool compare(const Order& a, const Order& b, std::function<bool(int, int)>& f)
{
    if (a.price == b.price)
    {
        if (a.timePlaced == b.timePlaced)
        {
            if (a.orderId < b.orderId)
                return true;
            else
                return false;
        }
        else
        {
            if (a.timePlaced < b.timePlaced)
                return true;
            else
                return false;
        }
    }
    else
    {
        if (f(a.price, b.price))
            return true;
        else
            return false;
    }
}

bool OrderBook::OrdCompSell::operator()(const Order& a, const Order& b) const
{
    std::function<bool(int, int)> comp = [](int a, int b)
    {
        return a < b;
    };
    return compare(a, b, comp);
}

bool OrderBook::OrdCompBuy::operator()(const Order& a, const Order& b) const
{
    std::function<bool(int, int)> comp = [](int a, int b)
    {
        return a > b;
    };
    return compare(a, b, comp);
}
