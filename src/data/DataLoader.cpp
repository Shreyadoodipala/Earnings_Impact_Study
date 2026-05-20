#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "../include/data/DataLoader.h"
#include "../include/common/utils.h"

std::map<std::string, Stock> loadStocks(const std::string& earningsFile, const std::string& etfFile) {
    // Step 1: Parse ETF file to create a map: ticker -> {name, sector}
    // Extract Columns: Ticker, Name, Sector
    std::map<std::string, std::pair<std::string,std::string>> etfData; // map: ticker -> (name, sector)

    std::ifstream etf(etfFile);
    if (!etf.is_open())
        throw std::runtime_error("Cannot open ETF file: " + etfFile);

    std::string line;
    std::getline(etf, line); // skip header

    while (std::getline(etf, line)) {
        if (line.empty()) continue;
        auto fields = splitCSVLine(line);
        if (fields.size() < 3) continue;

        std::string ticker = fields[0];  // col 0: Ticker
        std::string name   = fields[1];  // col 1: Name
        std::string sector = fields[2];  // col 2: Sector

        if (!ticker.empty())
            etfData[ticker] = {name, sector};
    }
    etf.close();

    // Step 2: Parse Earnings file → build Stock objects
    // Extract Columns: ticker, announcement date, estimated EPS, actual EPS, surprise%
    std::map<std::string, Stock> stocks; // map: ticker -> Stock (with name,sector, day0, estimatedEPS, actualEPS, surprisePct populated)

    std::ifstream earn(earningsFile);
    if (!earn.is_open())
        throw std::runtime_error("Cannot open earnings file: " + earningsFile);

    std::getline(earn, line); // skip header

    while (std::getline(earn, line)) {
        if (line.empty()) continue;
        auto fields = splitCSVLine(line);
        if (fields.size() < 5) continue;

        std::string ticker = fields[0]; // col 0: ticker
        std::string day0 = fields[1];   // col 1: announcement date  (YYYY-MM-DD)
        double estimatedEPS = 0.0, actualEPS = 0.0, surprisePct = 0.0;
        try {
            estimatedEPS = std::stod(fields[2]); // col 2: estimated EPS
            actualEPS = std::stod(fields[3]);   // col 3: actual EPS
            surprisePct = std::stod(fields[4]); // col 4: surprise%
        } catch (...) {
            std::cerr << "Bad EPS/surprise values for " << ticker << "\n";
        }

        // Look up name and sector from ETF file
        std::string name, sector;
        auto it = etfData.find(ticker);
        if (it != etfData.end()) {
            name   = it->second.first;
            sector = it->second.second;
        } else {
            name   = ticker;
            sector = "Unknown";
            std::cerr << "Warning: " << ticker << " not found in ETF holdings.\n";
        }

        stocks.emplace(ticker, Stock(ticker, name, sector, day0, estimatedEPS, actualEPS, surprisePct));
    }
    earn.close();

    return stocks;
}