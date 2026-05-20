#pragma once

#include <string>
#include <map>
#include "../common/utils.h"
#include "../common/Matrix.h"
#include <cmath>

class Stock {
    private:
        std::string ticker, name, sector;
        std::string day0; // announcement date in "YYYY-MM-DD" format
        double estimatedEPS, actualEPS, surprisePct;

        Group group;
        std::map<std::string, double> priceHistory; // date -> price
        Vector priceVector; // ordered vector of prices from priceHistory
        Vector benchmarkVector; // ordered vector of benchmark prices
        Vector returns, benchmarkReturns, cumDailyReturns, abnormalReturns;

    public:
        // constructors
        Stock(const std::string& ticker_, const std::string& name_, const std::string& sector_, const std::string& day0_, double estimatedEPS_, double actualEPS_, double surprisePct_) :
            ticker(ticker_), name(name_), sector(sector_), day0(day0_), estimatedEPS(estimatedEPS_), actualEPS(actualEPS_), surprisePct(surprisePct_), group(Group::Unassigned) {};
        Stock() : ticker(""), name(""), sector(""), day0(""), estimatedEPS(0), actualEPS(0), surprisePct(0), group(Group::Unassigned) {};
        Stock(const Stock& other) = default;

        // getters
        std::string getTicker() const;
        std::string getName() const;
        std::string getSector() const;
        std::string getAnnouncementDate() const;
        double getSurprisePct() const;
        Group getGroup() const;
        double getPrice(const std::string& date) const;
        Vector getPrices() const;
        Vector getBenchmarkPrices() const;
        const std::map<std::string, double>& getPriceHistory() const;
        const Vector& getReturns() const;
        const Vector& getBenchmarkReturns() const;

        // setters
        void addPrice(const std::string& date, double price);
        void setGroup(Group g);
        void constructPriceVector(); // call after all price data is added
        void setBenchmarkVector(const Vector& benchmark);
        void setReturns();

        // Other
        bool hasPrice(const std::string& date) const;
        void display() const;
};
