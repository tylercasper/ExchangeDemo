#ifndef TRADEENGINE_TYPES_H
#define TRADEENGINE_TYPES_H
#include <string>
#include <queue>
#include <vector>
#include <unordered_map>
#include <boost/thread.hpp>

class ThreadSafeQueue
{
public:
    void push(const std::string& str);
    bool empty();
    std::string pop_front();

private:
    mutable boost::mutex mutex;
    std::queue<std::string> queue;
};

class Order
{
public:
    Order(std::vector<std::string>);
    Order(int user, std::string& symb, int price, int quantity, char side, int user_ord_id);
    bool operator< (const Order& order) const;
    std::string tobOrdToString() const;
    std::string order_id;
    int user;
    std::string symbol;
    int price;
    int quantity;
    char side;
    int user_order_id;
    std::time_t time_placed;
};

class OrderBook
{
public:
    std::shared_ptr<Order> getOrder(std::string order_id);
    std::shared_ptr<Order> getTopMatchingOrder(Order& ord);
    std::string addOrderToBook(Order& ord);
    std::string removeOrderFromBook(std::string order_id);
    std::string removeOrderFromBook(const std::string& user_id, const std::string& user_order_id);
    void clear();
private:
    struct OrdCompSell
    {
        bool operator()(const Order& a, const Order& b) const;
    };
    struct OrdCompBuy
    {
        bool operator()(const Order& a, const Order& b) const;
    };
//    struct PriceIDKey{
//        int price;
//        int id;
//    };
    std::unordered_map<std::string, std::map<std::string, Order>> orderBookSell;
    std::map<std::string, std::map<Order, std::string, OrdCompSell>> orderBookSellInverse;
    std::unordered_map<std::string, std::map<std::string, Order>> orderBookBuy;
    std::map<std::string, std::map<Order, std::string, OrdCompBuy>> orderBookBuyInverse;
    std::map<std::string, std::string> order_book_lookup;
};


#endif //TRADEENGINE_TYPES_H
