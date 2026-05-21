#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "../common/Matrix.h"
#include "../core/Stock.h"
#include "../core/Sector.h"

struct BootstrappingResult 
{
    Vector eAAR;
    Vector stdAAR;
    Vector eCAAR;
    Vector stdCAAR; 
};

struct GroupResult 
{
    std::map<std::string, Stock> beatStock;
    std::map<std::string, Stock> meetStock;
    std::map<std::string, Stock> missStock;
    std::map<std::string, Stock> fullStockList; 
};

// get maps which contain stocks in beat/meet/miss group and their corresponding prices 
GroupResult grouping(std::map<std::string, Sector> allStocks);

std::vector<const Stock*> random30Stocks(const std::map<std::string, Stock>& stockList);

BootstrappingResult bootstrapping(const std::map<std::string, Stock>& stockList);

Vector tStat(std::vector<double> e, std::vector<double> std); 

struct CIResult
{
    Vector upper;
    Vector lower;
};

CIResult CI(Vector e, Vector std); 

// show the statistical significance table 
void display(int N, BootstrappingResult bResult, std::string option);
