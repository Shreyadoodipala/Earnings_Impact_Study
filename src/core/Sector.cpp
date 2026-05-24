#include "../include/core/Sector.h"
#include "../include/core/Stock.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>

void Sector::addStock(const Stock& stock) {
    stocks.push_back(stock);
    total++;
}

void Sector::sortStocksBySurprise() {
    std::sort(stocks.begin(), stocks.end(), [](const Stock& a, const Stock& b) {
        return a.getSurprisePct() > b.getSurprisePct();
    });
}

void Sector::removeOutliers(double pct) {
    if (stocks.empty()) return;

    int numToRemove = static_cast<int>(stocks.size() * pct);
    for (int i = 0; i < numToRemove; i++) {
        stocks.erase(stocks.begin()); // remove from front
        stocks.pop_back(); // remove from back
        exclude += 2;
    }
}

std::string Sector::getSector() const {
    return sector;
}

void Sector::setGroup() {
    int remaining = total - exclude;
    if (remaining == 0) return;
    int groupSize = remaining / 3;
    int remainder = remaining % 3;
    
    beatCount = groupSize + (remainder > 0 ? 1 : 0);
    meetCount = groupSize + (remainder > 1 ? 1 : 0);
    missCount = groupSize;

    for (int i = 0; i < remaining; ++i) {
        if (i < beatCount) stocks[i].setGroup(Group::Beat);
        else if (i < beatCount + meetCount) stocks[i].setGroup(Group::Meet);
        else stocks[i].setGroup(Group::Miss);
    }
    
}

std::vector<Stock>& Sector::getStocks() {
    return stocks;
}

int Sector::getTotal() const { return total; }
int Sector::getExclude() const { return exclude; }
int Sector::getBeatCount() const { return beatCount; }
int Sector::getMeetCount() const { return meetCount; }
int Sector::getMissCount() const { return missCount; }

void Sector::sector_summary() const {
    std::cout << std::left << std::setw(35) << sector 
              << std::right << std::setw(10) << total 
              << std::right << std::setw(10) << exclude 
              << std::right << std::setw(10) << (total - exclude) 
              << std::right << std::setw(10) << beatCount 
              << std::right << std::setw(10) << meetCount 
              << std::right << std::setw(10) << missCount 
              << std::endl;
}

void Sector::sector_summary(std::ofstream& outFile) const {
    outFile << std::left << std::setw(35) << sector 
            << std::right << std::setw(10) << total 
            << std::right << std::setw(10) << exclude 
            << std::right << std::setw(10) << (total - exclude) 
            << std::right << std::setw(10) << beatCount 
            << std::right << std::setw(10) << meetCount 
            << std::right << std::setw(10) << missCount 
            << std::endl;
}

void all_sectors_summary(std::map<std::string, Sector> s) {
    std::vector<Sector> sectors;
    for (const auto& [name, sector] : s) {
        sectors.push_back(sector);
    }

    std::cout << std::string(105, '-') << std::endl;
    std::cout << std::left << std::setw(35) << "Sector" 
              << std::right << std::setw(10) << "Total" 
              << std::right << std::setw(10) << "Excluded" 
              << std::right << std::setw(10) << "Included" 
              << std::right << std::setw(10) << "Beat" 
              << std::right << std::setw(10) << "Meet" 
              << std::right << std::setw(10) << "Miss" 
              << std::endl;
    int totalAll = 0, excludeAll = 0, beatAll = 0, meetAll = 0, missAll = 0;
    std::cout << std::string(105, '-') << std::endl;
    for (const auto& sector : sectors) {
        sector.sector_summary();
        totalAll += sector.getTotal();
        excludeAll += sector.getExclude();
        beatAll += sector.getBeatCount();
        meetAll += sector.getMeetCount();
        missAll += sector.getMissCount();
    }
    int includedAll = totalAll - excludeAll;
    std::cout << std::string(105, '-') << std::endl;
    std::cout << std::left << std::setw(35) << "All Sectors" 
              << std::right << std::setw(10) << totalAll 
              << std::right << std::setw(10) << excludeAll 
              << std::right << std::setw(10) << includedAll 
              << std::right << std::setw(10) << beatAll 
              << std::right << std::setw(10) << meetAll 
              << std::right << std::setw(10) << missAll 
              << std::endl;
    std::cout << std::string(105, '-') << std::endl;

}

void all_sectors_summary(std::map<std::string, Sector> s, std::ofstream& outFile) {
    std::vector<Sector> sectors;
    for (const auto& [name, sector] : s) {
        sectors.push_back(sector);
    }

    outFile << std::string(105, '-') << std::endl;
    outFile << std::left << std::setw(35) << "Sector" 
            << std::right << std::setw(10) << "Total" 
            << std::right << std::setw(10) << "Excluded" 
            << std::right << std::setw(10) << "Included" 
            << std::right << std::setw(10) << "Beat" 
            << std::right << std::setw(10) << "Meet" 
            << std::right << std::setw(10) << "Miss" 
            << std::endl;
    int totalAll = 0, excludeAll = 0, beatAll = 0, meetAll = 0, missAll = 0;
    outFile << std::string(105, '-') << std::endl;
    for (const auto& sector : sectors) {
        sector.sector_summary(outFile);
        totalAll += sector.getTotal();
        excludeAll += sector.getExclude();
        beatAll += sector.getBeatCount();
        meetAll += sector.getMeetCount();
        missAll += sector.getMissCount();
    }
    int includedAll = totalAll - excludeAll;
    outFile << std::string(105, '-') << std::endl;
    outFile << std::left << std::setw(35) << "All Sectors" 
            << std::right << std::setw(10) << totalAll 
            << std::right << std::setw(10) << excludeAll 
            << std::right << std::setw(10) << includedAll 
            << std::right << std::setw(10) << beatAll 
            << std::right << std::setw(10) << meetAll 
            << std::right << std::setw(10) << missAll 
            << std::endl;
    outFile << std::string(105, '-') << std::endl;
}

std::map<std::string, Sector> groupStocks(std::map<std::string, Stock>& stocks) {
    // Step 1: Group stocks by sector
    std::map<std::string, Sector> sectors; // map: sector -> Sector object
    for (const auto& [ticker, stock] : stocks) {
        std::string sectorName = stock.getSector();
        auto [it, inserted] = sectors.emplace(sectorName, Sector(sectorName));
        it->second.addStock(stock);
    }

    // Step 2: For each sector, sort stocks by surprise%, remove outliers, and assign groups
    for (auto& [sectorName, sector] : sectors) {
        sector.sortStocksBySurprise();
        sector.removeOutliers(0.02); // remove top and bottom 2%
        sector.setGroup();
    }

    // Step 3: Update original stocks with their assigned groups
    for (auto& [sectorName, sector] : sectors) {
        for (const auto& stock : sector.getStocks()) {
            stocks.insert_or_assign(stock.getTicker(), stock); // update with group assignment
        }
    }
    return sectors;
}