cmake_minimum_required(VERSION 3.22)
set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)
project(TradeEngine VERSION 0.1)
find_package(Boost REQUIRED COMPONENTS system thread chrono regex)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(TradeEngine main.cpp Server.cpp Server.h TestClient.cpp TestClient.h Types.cpp Types.h TradeEngine.cpp TradeEngine.h)
target_include_directories(TradeEngine PUBLIC
        ${PROJECT_BINARY_DIR}
        )
configure_file(TradeEngineConfig.h.in TradeEngineConfig.h)
target_link_libraries(TradeEngine Boost::boost Boost::thread Boost::chrono)

