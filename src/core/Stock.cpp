#include <stdexcept>
#include <iostream>
#include "../include/core/Stock.h"

// getters
std::string Stock::getTicker() const { return ticker; }
std::string Stock::getName() const { return name; }
std::string Stock::getSector() const { return sector; }
std::string Stock::getAnnouncementDate() const { return day0; }
double Stock::getSurprisePct() const { return surprisePct; }
Group Stock::getGroup() const { return group; }

double Stock::getPrice(const std::string& date) const {
    auto it = priceHistory.find(date);
    if (it != priceHistory.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Price for date " + date + " not found.");
    }
}
std::vector<std::string> Stock::getPriceDates() const {
    std::vector<std::string> dates;
    dates.reserve(priceHistory.size());
    for (const auto& [date, price] : priceHistory)
        dates.push_back(date);
    return dates;
}

Vector Stock::getPrices() const { return priceVector; }
Vector Stock::getBenchmarkPrices() const { return benchmarkVector; }
const std::map<std::string, double>& Stock::getPriceHistory() const { return priceHistory; }
const Vector& Stock::getReturns() const { return returns; }
const Vector& Stock::getBenchmarkReturns() const { return benchmarkReturns; }

// setters
void Stock::addPrice(const std::string& date, double price) { priceHistory[date] = price; }
void Stock::setGroup(Group g) { group = g; }

void Stock::constructPriceVector() {
    priceVector.clear();
    for (const auto& pair : priceHistory) {
        priceVector.push_back(pair.second);
    }
}
void Stock::setBenchmarkVector(const Vector& benchmark) { benchmarkVector = benchmark; }

void Stock::setReturns() { 
    returns = dailyReturns(priceVector);
    benchmarkReturns = dailyReturns(benchmarkVector);
}

// Other
bool Stock::hasPrice(const std::string& date) const { 
    return priceHistory.find(date) != priceHistory.end(); 
}

void Stock::display() const {
    std::cout << "Ticker: " << ticker << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Sector: " << sector << std::endl;
    std::cout << "Announcement Date: " << day0 << std::endl;
    std::cout << "Estimated EPS: " << estimatedEPS << std::endl;
    std::cout << "Actual EPS: " << actualEPS << std::endl;
    std::cout << "Surprise %: " << surprisePct << std::endl;
    std::cout << "Group: " << groupToString(group) << std::endl;
}