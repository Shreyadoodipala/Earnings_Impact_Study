#include "../include/analysis/Calculation.h"
#include <random>
#include <algorithm>
#include <map>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <omp.h>

GroupResult grouping(std::map<std::string, Sector>& allStocks)
{
    std::map<std::string, Stock> beatStock, meetStock, missStock, fullStock;
    for (auto& sector : allStocks)
    {
        std::vector<Stock> stockList = sector.second.getStocks();
        for (auto s: stockList)
        {
            fullStock.emplace(s.getTicker(), s);
            if (s.getGroup() == Group::Beat) { beatStock.emplace(s.getTicker(), s); }
            else if (s.getGroup() == Group::Meet) { meetStock.emplace(s.getTicker(), s); }
            else if (s.getGroup() == Group::Miss) { missStock.emplace(s.getTicker(), s); }
        }
    }
    GroupResult result; 
    result.beatStock = beatStock;
    result.meetStock = meetStock;
    result.missStock = missStock;
    result.fullStockList = fullStock;

    return result;
}

std::vector<const Stock*> random30Stocks(const std::map<std::string, Stock>& stockList)
{
    std::vector<const Stock*> resultPtr; 
    resultPtr.reserve(stockList.size());
    for (const auto& [ticker, stock] : stockList)
    {
        resultPtr.push_back(&stock);
    }
    
    std::random_device random;
    std::mt19937 generator(random());
    std::shuffle(resultPtr.begin(), resultPtr.end(), generator);
    resultPtr.resize(30); 

    return resultPtr;
}

BootstrappingResult bootstrapping(const std::map<std::string, Stock>& stockList)
{
    Matrix AAR; 
    Matrix CAAR; 

    // repeat every step 50 times to create 50 samplings (parallelized across threads)
    #pragma omp parallel for
    for (int i = 0; i < 50; i++)
    {
        // randomly select 30 stocks 
        std::vector<const Stock*> selectedList = random30Stocks(stockList);

        Matrix ARit; 
        for (const auto &s : selectedList)
        {
            const Vector& Rmt = s->getBenchmarkReturns();
            const Vector& Rit = s->getReturns();

            Vector AR = Rit - Rmt;
            ARit.push_back(AR);
        }

        Vector AARt = colMean(ARit);
        Vector CAARt = calcCAAR(AARt);
        
        // Thread-safe aggregation of results
        #pragma omp critical
        {
            AAR.push_back(AARt);
            CAAR.push_back(CAARt);
        }
    }

    // calculate average and std 
    BootstrappingResult result; 
    result.eAAR = colMean(AAR);
    result.eCAAR = colMean(CAAR);
    result.stdAAR = colStd(AAR);
    result.stdCAAR = colStd(CAAR);

    return result; 
}

Vector tStat(Vector e, Vector std) { return e / std; }

CIResult CI(Vector e, Vector std)
{
    CIResult result;
    result.lower = e - 1.96 * std;
    result.upper = e + 1.96 * std;

    return result; 
}

// 'option' should be either "AAR" or "CAAR"
void display(int N, BootstrappingResult bResult, std::string option)
{
    if (option != "AAR" && option != "CAAR")
    {
        std::cout << "Invalid option. Option can only be AAR or CAAR." << std::endl;
        return; 
    }

    Vector tResult;
    CIResult cResult;
    if (option == "AAR") 
    {
        tResult = tStat(bResult.eAAR, bResult.stdAAR);
        cResult = CI(bResult.eAAR, bResult.stdAAR);
        
        std::cout << std::fixed << std::setprecision(4); 
        std::cout << std::setw(6) << "Day" << std::setw(12) << "E[AAR(t)]" << std::setw(12) << "AAR-STD(t)"
            << std::setw(12) << "t-stat" << std::setw(12) << "95% CI Lower" << std::setw(12) << "95% CI Upper" << std::endl;
        for (int i = 0; i < 2 * N; i++)
        {
            std::cout << std::setw(6) << i - N + 1 << std::setw(12) << bResult.eAAR[i] << std::setw(12) << bResult.stdAAR[i] 
                    << std::setw(12) << tResult[i] << std::setw(12) << cResult.lower[i] << std::setw(12) << cResult.upper[i] << std::endl;
        }
    }
    else 
    {
        tResult = tStat(bResult.eCAAR, bResult.stdCAAR);
        cResult = CI(bResult.eCAAR, bResult.stdCAAR); 

        std::cout << std::fixed << std::setprecision(4); 
        std::cout << std::setw(6) << "Day" << std::setw(12) << "E[CAAR(t)]" << std::setw(12) << "CAAR-STD(t)"
                << std::setw(12) << "t-stat" << std::setw(12) << "95% CI Lower" << std::setw(12) << "95% CI Upper" << std::endl;
        for (int i = 0; i < 2 * N; i++)
        {
            std::cout << std::setw(6) << i - N + 1 << std::setw(12) << bResult.eCAAR[i] << std::setw(12) << bResult.stdCAAR[i] 
                    << std::setw(12) << tResult[i] << std::setw(12) << cResult.lower[i] << std::setw(12) << cResult.upper[i] << std::endl;
        }
    }
}

void display(int N, BootstrappingResult bResult, std::string option, std::ofstream& outFile)
{
    if (option != "AAR" && option != "CAAR")
    {
        outFile << "Invalid option. Option can only be AAR or CAAR." << std::endl;
        return; 
    }

    Vector tResult;
    CIResult cResult;
    if (option == "AAR") 
    {
        tResult = tStat(bResult.eAAR, bResult.stdAAR);
        cResult = CI(bResult.eAAR, bResult.stdAAR);
        
        outFile << std::fixed << std::setprecision(4); 
        outFile << std::setw(6) << "Day" << std::setw(12) << "E[AAR(t)]" << std::setw(12) << "AAR-STD(t)"
            << std::setw(12) << "t-stat" << std::setw(12) << "95% CI Lower" << std::setw(12) << "95% CI Upper" << std::endl;
        for (int i = 0; i < 2 * N; i++)
        {
            outFile << std::setw(6) << i - N + 1 << std::setw(12) << bResult.eAAR[i] << std::setw(12) << bResult.stdAAR[i] 
                    << std::setw(12) << tResult[i] << std::setw(12) << cResult.lower[i] << std::setw(12) << cResult.upper[i] << std::endl;
        }
    }
    else 
    {
        tResult = tStat(bResult.eCAAR, bResult.stdCAAR);
        cResult = CI(bResult.eCAAR, bResult.stdCAAR); 

        outFile << std::fixed << std::setprecision(4); 
        outFile << std::setw(6) << "Day" << std::setw(12) << "E[CAAR(t)]" << std::setw(12) << "CAAR-STD(t)"
                << std::setw(12) << "t-stat" << std::setw(12) << "95% CI Lower" << std::setw(12) << "95% CI Upper" << std::endl;
        for (int i = 0; i < 2 * N; i++)        {
            outFile << std::setw(6) << i - N + 1 << std::setw(12) << bResult.eCAAR[i] << std::setw(12) << bResult.stdCAAR[i] 
                    << std::setw(12) << tResult[i] << std::setw(12) << cResult.lower[i] << std::setw(12) << cResult.upper[i] << std::endl;
        }
    }
}