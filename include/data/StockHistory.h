#pragma once

#include <map>
#include <string>
#include <vector>
#include "../core/Stock.h"

struct PreparedEventWindow {
    std::string ticker;
    std::string announcementDate, eventDate;
    int eventIndex, eventWindowIndex;
    std::vector<std::string> dates;
    Vector stockPrices, benchmarkPrices;
    std::vector<std::string> warnings;
    bool valid;

    PreparedEventWindow();
    void setBenchmarkPrices(const Vector& prices);
    void setBenchmarkPrices(Vector&& prices);
};

struct StockPrepSummary {
    std::map<std::string, PreparedEventWindow> validWindows;
    std::map<std::string, std::vector<std::string>> skippedStocks;
};

int findEventDayIndex(const Stock &stock, std::vector<std::string> &warnings);

PreparedEventWindow prepareEventWindow(const Stock &stock, const std::map<std::string, double>&iwvPrices, 
    int N);

StockPrepSummary prepareEventWindows(const std::map<std::string, Stock> &stocks, 
    const std::map<std::string, double> &iwvPrices, int N);

// Subset of `universe` containing only tickers in summary.validWindows (full Stock metadata for groupStocks).
std::map<std::string, Stock> stocksForValidWindows(const std::map<std::string, Stock> &universe, 
    const StockPrepSummary &summary);

std::vector<std::string> populatePriceHistoryFromEODHD(
    std::map<std::string, Stock> &stocks,
    std::map<std::string, double> &benchmarkPrices, const std::string &benchmarkTicker,
    int N, const std::string &apiToken);