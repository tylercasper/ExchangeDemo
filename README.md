Kraken Exchange Demo
====================

C++ Build
---------

To build from source, the following tools/libraries are needed:

  * cmake
  * make
  * g++/clang++
  * boost

To build and run the exchange demo, execute the following:

    git clone https://github.com/tylercasper/KrakenExchangeDemo.git
    cd KrakenExchangeDemo
    mkdir build && cd build
    cmake ..
    make
    ./TradeEngine

TODO:
----
  * unit tests
  * performance tests
  * documentation
  * containerization
  * binary wire protocol
