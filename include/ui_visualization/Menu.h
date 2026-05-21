#pragma once
#include <map>
#include <string>
#include <iostream>
#include "../core/Stock.h"
#include "../core/Sector.h"
#include "../common/Matrix.h"
#include "../analysis/Calculation.h"
#include "../data/StockHistory.h"

class Menu {
private:
    int N;                          // Event window size (minimum 30)
    bool isDataProcessed;           // Flag to check if Option 1 has been executed
    std::map<std::string, Stock> allStocks;
    std::map<std::string, Stock> validStocks;
    std::map<std::string, Sector> sectorMap;
    std::map<std::string, double> benchmarkPrices;
    StockPrepSummary summary;       // Summary of valid and skipped stocks
    GroupResult groupedData;        // Store processed groups (Beat/Meet/Miss)
    std::map<std::string, BootstrappingResult> bootResults;   // Bootstrapping results for each group
    // std::vector<std::vector<Vector>> resultMatrix; 

    // Private handler functions for each menu option
    void handleOption1();           // Retrieve data & Grouping
    void handleOption2();           // Individual Stock Lookup
    void handleOption3();           // Stats for a Group
    void handleOption4();           // Gnuplot Visualization
    void handleOption5();           // Sector Summary Report

    // Utility function
    void clearInputBuffer();

public:
    Menu();
    void run();                     // Start the menu loop
};