#pragma once

#include <string>
#include <map>
#include "../core/Stock.h"

// Returns a map of ticker -> Stock, populated with name, sector, day0, and surprisePct
std::map<std::string, Stock> loadStocks(const std::string& earningsFile, const std::string& etfFile);