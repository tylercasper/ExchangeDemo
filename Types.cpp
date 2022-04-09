#include "Types.h"
#include <iostream>
using std::stoi;

void ThreadSafeQueue::push(const std::string& str)
{
    boost::mutex::scoped_lock lock(mutex);
    queue.push(str);
}

bool ThreadSafeQueue::empty()
{
    boost::mutex::scoped_lock lock(mutex);
    return queue.empty();
}

std::string ThreadSafeQueue::pop_front()
{
    boost::mutex::scoped_lock lock(mutex);
    auto str = queue.front();
    queue.pop();
    return str;
}

Order::Order(int user, std::string& symb, int price, int quantity, char side, int user_ord_id):
user(user), symbol(symb), price(price), quantity(quantity), side(side), user_order_id(user_ord_id),
order_id(std::string(std::to_string(user) + "." + std::to_string(user_ord_id)))
{}

Order::Order(std::vector<std::string> ord_str):
    Order(stoi(ord_str[0]), ord_str[1], stoi(ord_str[2]), stoi(ord_str[3]),
          (char)ord_str[4][0], stoi(ord_str[5]))
{}

std::string Order::tobOrdToString() const
{
    return std::string("B, " + std::string(1, side) + ", " + std::to_string(price) + ", " + std::to_string(quantity));

}

bool Order::operator<(const Order &order) const
{
    if (order.order_id < this->order_id)
        return true;
}

std::shared_ptr<Order> OrderBook::getOrder(std::string order_id)
{
    std::string book = order_book_lookup[order_id];
    auto it = orderBookSell[book].find(order_id);
    if (it != orderBookSell[book].end())
        return std::make_shared<Order>(it->second);
    else if (it = orderBookBuy[book].find(order_id); it != orderBookBuy[book].end())
        return std::make_shared<Order>(it->second);

    return nullptr;
}

std::shared_ptr<Order> OrderBook::getTopMatchingOrder(Order &ord)
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

std::string OrderBook::addOrderToBook(Order &ord)
{
//    std::cout << "Engine: adding order to book: " << ord.tobOrdToString() << std::endl;

    std::string top;
    order_book_lookup[ord.order_id] = ord.symbol;
    if (ord.side == 'B')
    {
        orderBookBuy[ord.symbol].insert({ord.order_id, ord});
//            std::cout << "Insertion into orderBookBuy failed" << std::endl;
        orderBookBuyInverse[ord.symbol].insert({ord, ord.order_id});
//            std::cout << "Insertion into orderBookBuyInverse failed" << std::endl;
        if (ord.order_id == orderBookBuyInverse[ord.symbol].begin()->second)
            top = orderBookBuyInverse[ord.symbol].begin()->first.tobOrdToString();
//        for (auto& [key, val] : orderBookBuyInverse[ord.symbol])
//            std::cout << key.order_id << ", " << key.price << std::endl;
//        std::cout << "Engine: added order to book: " << ord.tobOrdToString() << std::endl;
    }
    else
    {
        orderBookSell[ord.symbol].insert({ord.order_id, ord});
        orderBookSellInverse[ord.symbol].insert({ord, ord.order_id});
        if (ord.order_id == orderBookSellInverse[ord.symbol].begin()->second)
            top = orderBookSellInverse[ord.symbol].begin()->first.tobOrdToString();
//        for (auto& [key, val] : orderBookSellInverse[ord.symbol])
//            std::cout << key.order_id << ", " << key.price << std::endl;
//        std::cout << "Engine: added order to book: " << ord.tobOrdToString() << std::endl;
    }
    return top;
}

std::string OrderBook::removeOrderFromBook(std::string order_id)
{
//    std::cout << "Engine: removing order from book: " << order_id << std::endl;
    std::string top;
    std::string book = order_book_lookup[order_id];
    auto it = orderBookSell[book].find(order_id);
    if (it != orderBookSell[book].end())
    {
        if (it->second.order_id == orderBookSellInverse[book].begin()->second)
            top = (++orderBookSellInverse[book].begin())->first.tobOrdToString();

//        std::cout << "Engine: removed order to book: " << it->second.tobOrdToString()  << ", " << it->second.order_id << std::endl;
        orderBookSellInverse[book].erase(it->second);
        orderBookSell[book].erase(order_id);
//        for (auto& [key, val] : orderBookSellInverse[book])
//            std::cout << key.order_id << ", " << key.price << std::endl;
    }
    else if (it = orderBookBuy[book].find(order_id); it != orderBookBuy[book].end())
    {
        if (it->second.order_id == orderBookBuyInverse[book].begin()->second)
            top = (++orderBookBuyInverse[book].begin())->first.tobOrdToString();
//        std::cout << "Engine: removed order to book: " << it->second.tobOrdToString()  << ", " << it->second.order_id << std::endl;
        orderBookBuyInverse[book].erase(it->second);
        orderBookBuy[book].erase(order_id);
//        for (auto& [key, val] : orderBookBuyInverse[book])
//            std::cout << key.order_id << ", " << key.price << std::endl;
    }

    return top;
}

void OrderBook::clear()
{
    for (auto &[key, value]: orderBookBuy)
        value.clear();
    for (auto &[key, value]: orderBookBuyInverse)
        value.clear();
    for (auto &[key, value]: orderBookSell)
        value.clear();
    for (auto &[key, value]: orderBookSellInverse)
        value.clear();
}

std::string OrderBook::removeOrderFromBook(const std::string& user_id, const std::string& user_order_id)
{
    return removeOrderFromBook(user_id + "." + user_order_id);
}

bool OrderBook::OrdCompSell::operator()(const Order& a, const Order& b) const
{
    if (a.price == b.price)
    {
        if (a.time_placed == b.time_placed)
        {
            if (a.order_id < b.order_id)
                return true;
            else
                return false;
        }
        else
        {
            if (a.time_placed < b.time_placed)
                return true;
            else
                return false;
        }
    }
    else
    {
        if (a.price < b.price)
            return true;
        else
            return false;
    }
}

bool OrderBook::OrdCompBuy::operator()(const Order& a, const Order& b) const
{
    if (a.price == b.price)
    {
        if (a.time_placed == b.time_placed)
        {
            if (a.order_id < b.order_id)
                return true;
            else
                return false;
        }
        else
        {
            if (a.time_placed < b.time_placed)
                return true;
            else
                return false;
        }
    }
    else
    {
        if (a.price > b.price)
            return true;
        else
            return false;
    }
}
