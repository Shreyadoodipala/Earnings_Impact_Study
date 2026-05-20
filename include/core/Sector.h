# pragma once
#include "Stock.h"

class Sector {
    private:
        std::string sector;
        int total, exclude;
        int beatCount, meetCount, missCount;
        std::vector<Stock> stocks;
    public:
        Sector(std::string sector_): sector(sector_), total(0), exclude(0), beatCount(0), meetCount(0), missCount(0) {};
        Sector() : sector(""), total(0), exclude(0), beatCount(0), meetCount(0), missCount(0) {}
        Sector(const Sector& other) = default;

        void addStock(const Stock& stock);
        std::vector<Stock>& getStocks();
        void sortStocksBySurprise();
        void removeOutliers(double pct = 0.02);
        std::string getSector() const;
        void setGroup();
        void sector_summary() const;

        int getTotal() const;
        int getExclude() const;
        int getBeatCount() const;
        int getMeetCount() const;
        int getMissCount() const;
};
void all_sectors_summary(std::map<std::string, Sector> s);

// Group stocks by sector, remove outliers and assign each stock to Beat/Meet/Miss group within its sector
std::map<std::string, Sector> groupStocks(std::map<std::string, Stock>& stocks);