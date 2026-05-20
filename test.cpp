#include "include/data/DataLoader.h"
#include "include/core/Stock.h"
#include <iostream>

int main() {
    try {
        std::cout << "Loading stocks from CSV files...\n";
        auto stocks = loadStocks("data/earnings_cleaned.csv", "data/iShares-Russell-3000-ETF_fund_cleaned.csv");
        std::cout << "Loaded " << stocks.size() << " stocks\n";
        
        std::cout << "Looking for NVDA...\n";
        const Stock& nvda = stocks.at("NVDA");
        std::cout << "Found NVDA, displaying...\n";
        nvda.display();
        std::cout << "Done.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
}