#ifndef TRADEENGINE_TRADEENGINE_H
#define TRADEENGINE_TRADEENGINE_H
#include "Types.h"
#include "Server.h"
#include <map>
#include <unordered_map>

class TradeEngine {
public:
    TradeEngine(std::shared_ptr<Server> server);
    void run();
private:
    void processNewOrder(std::string& order);
    void processCancelOrder(std::string& order);
    void takeOrder(Order& order, std::shared_ptr<Order> match);
    void addToBook(Order& order);
    OrderBook orderBook;
    std::shared_ptr<Server> server;
};

#endif //TRADEENGINE_TRADEENGINE_H
