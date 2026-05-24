#include "../include/ui_visualization/Menu.h"
#include <iomanip>
#include <algorithm>
#include <cctype>   
#include <fstream>
#include "../include/data/DataLoader.h"
#include "../include/ui_visualization/Gnuplot.h"

static std::string toLower(std::string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

Menu::Menu() : N(0), isDataProcessed(false) {}

void Menu::run() {
    int choice = 0;
    std::cout << "\n==================================================" << std::endl;
    std::cout << "   Main Menu - Earnings Impact Study" << std::endl;
    std::cout << "==================================================" << std::endl;

    std::cout << "Do you want to save the results to a file? (y/n): ";
    char saveChoice;
    std::cin >> saveChoice;
    if (saveChoice == 'n' || saveChoice == 'N') {
        save = false;
        std::cout << "Results will not be saved to a file." << std::endl;

    } else if (saveChoice == 'y' || saveChoice == 'Y') {
        save = true;
        std::cout << "Results will be saved to a file." << std::endl;
    } else {
        std::cout << "Invalid choice. Defaulting to not saving results." << std::endl;
        save = false;
    }

    while (true) {
        std::cout << "\n==================================================" << std::endl;
        std::cout << "   Main Menu - Earnings Impact Study" << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << "1. Enter N to retrieve data & group stocks" << std::endl;
        std::cout << "2. Pull information for one stock" << std::endl;
        std::cout << "3. Show AAR/CAAR Statistics for one group" << std::endl;
        std::cout << "4. Plot CAAR for all groups (Gnuplot)" << std::endl;
        std::cout << "5. Show Sector Summary Report" << std::endl;
        std::cout << "6. Exit" << std::endl;
        std::cout << "Selection: ";

        if (!(std::cin >> choice)) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            clearInputBuffer();
            continue;
        }

        if (choice == 6) {
            std::cout << "Exiting program. Goodbye!" << std::endl;
            break;
        }

        switch (choice) {
            case 1: handleOption1(); break;
            case 2: handleOption2(); break;
            case 3: handleOption3(); break;
            case 4: handleOption4(); break;
            case 5: handleOption5(); break;
            default: std::cout << "Invalid selection! Please try again." << std::endl;
        }
    }
}

void Menu::handleOption1() {
    std::cout << "Enter N (N >= 30): ";
    std::cin >> N;
    if (N < 30) {
        std::cout << "Error: N must be at least 30 for significant results." << std::endl;
        return;
    }

    std::string earningsFile = "data/earnings_cleaned.csv";
    std::string etfFile = "data/iShares-Russell-3000-ETF_fund_cleaned.csv";

    allStocks = loadStocks(earningsFile, etfFile);
    if (allStocks.empty()) {
        std::cout << "Error: No stocks loaded. Please check your CSV files." << std::endl;
        return;
    }

    std::string apiToken;
    std::ifstream tokenFile("api_key.txt");
    if (!tokenFile) {
        std::cout << "Error: Cannot open api_key.txt" << std::endl;
        return;
    }
    std::getline(tokenFile, apiToken);

    std::cout << "Fetching price data from EODHD..." << std::endl;

    std::vector<std::string> warnings = populatePriceHistoryFromEODHD(allStocks, benchmarkPrices, "IWV", N, apiToken);

    for (const auto& w : warnings) { std::cout << "[Warning] " << w << std::endl; }

    summary = prepareEventWindows(allStocks, benchmarkPrices, N);
    for (auto& [ticker, window] : summary.validWindows) {
    auto it = allStocks.find(ticker);
    if (it != allStocks.end()) {
        it->second.setPriceVector(window.stockPrices);
        it->second.setBenchmarkVector(window.benchmarkPrices);
        it->second.setReturns();
    }
}

    std::cout << "Valid stocks: " << summary.validWindows.size() << std::endl;
    std::cout << "Skipped stocks: " << summary.skippedStocks.size() << std::endl;

    if (save == true) {
        std::ofstream outFile("results/Results_Summary.txt");
        if (!outFile) {
            std::cout << "Error: Cannot create results/Results_Summary.txt" << std::endl;
            return;
        }
        outFile << "Window size (N): " << N << std::endl << std::endl;
        outFile << "Stock Preparation Summary" << std::endl;
        outFile << "=========================" << std::endl;
        outFile << "Valid Stocks: " << summary.validWindows.size() << std::endl;
        outFile << "Skipped Stocks: " << summary.skippedStocks.size() << std::endl << std::endl;
        outFile << "Details of Skipped Stocks can be found in logs.txt" << std::endl;

        std::ofstream logFile("results/logs.txt", std::ios::app);
        if (!logFile) {
            std::cout << "Error: Cannot create results/logs.txt" << std::endl;
            return;
        }
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        logFile << "Log Timestamp: " << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
        for (const auto& [ticker, reasons] : summary.skippedStocks) {
            logFile << "Ticker: " << ticker << std::endl;
            for (const auto& reason : reasons) {
                logFile << "  - " << reason << std::endl;
            }
            logFile << std::endl;
        }
        outFile.close();
        logFile.close();
    }

    validStocks = stocksForValidWindows(allStocks, summary);

    sectorMap = groupStocks(validStocks);
    
    isDataProcessed = true;
    std::cout << "Data retrieval and grouping completed successfully." << std::endl;
}

void Menu::handleOption2() {
    if (!isDataProcessed) {
        std::cout << "Error: Please run Option 1 to initialize data first." << std::endl;
        return;
    }

    std::string ticker;
    std::cout << "Enter Stock Ticker: ";
    std::cin >> ticker;

    auto it = validStocks.find(ticker);
    if (it != validStocks.end()) {
        it->second.display(); 
        if (save == true) {
            std::ofstream outFile("results/Results_Summary.txt", std::ios::app);
            if (outFile) {
                outFile << "\n--- Details for Stock: " << ticker << " ---\n";
                it->second.display(outFile);
                outFile.close();
            }
        }
    } else {
        std::cout << "Ticker [" << ticker << "] not found or was excluded as an outlier." << std::endl;
    }
}

void Menu::handleOption3() {
    if (!isDataProcessed) {
        std::cout << "Error: No data available. Run Option 1 first." << std::endl;
        return;
    }

    groupedData = grouping(sectorMap);
    
    bootResults["beat"] = bootstrapping(groupedData.beatStock);
    bootResults["meet"] = bootstrapping(groupedData.meetStock);
    bootResults["miss"] = bootstrapping(groupedData.missStock);

    std::string groupName;
    std::cout << "Select Group to Display (Beat/Meet/Miss): ";
    std::cin >> groupName;
    groupName = toLower(groupName);

    if (bootResults.find(groupName) != bootResults.end()) {
        display(N, bootResults[groupName], "AAR");
        display(N, bootResults[groupName], "CAAR");

        if (save == true) {
            std::ofstream outFile("results/Results_Summary.txt", std::ios::app);
            if (outFile) {
                outFile << "\n--- Statistics for Group: " << groupName << " ---\n";
                outFile << "\nAAR Statistics:\n";
                display(N, bootResults[groupName], "AAR", outFile);
                outFile << "\nCAAR Statistics:\n";
                display(N, bootResults[groupName], "CAAR", outFile);
                outFile.close();
            }
        }
    }
    else {
        std::cout << "Invalid group name!" << std::endl;
    }
}

void Menu::handleOption4() {
    if (!isDataProcessed) {
        std::cout << "Error: Please initialize data (Option 1) before plotting." << std::endl;
        return;
    }

    if (bootResults.find("beat") == bootResults.end() ||
        bootResults.find("meet") == bootResults.end() ||
        bootResults.find("miss") == bootResults.end()) {
        std::cout << "Error: Bootstrapping results not available. Run Option 3 first." << std::endl;
        return;
    }

    int size = bootResults["beat"].eCAAR.size();
    if (size == 0) {
        std::cout << "No CAAR data to plot." << std::endl;
        return;
    }

    // x-axis: [-N, ..., 0, ..., +N]
    Vector xData(size);
    for (int i = 0; i < size; ++i) {
        xData[i] = i - (size / 2);
    }

    Matrix yData = {
        bootResults["beat"].eCAAR,
        bootResults["meet"].eCAAR,
        bootResults["miss"].eCAAR
    };  

    Gnuplot plot("Expected CAAR Comparison for Beat, Meet, and Miss", "Event Time (days)", "CAAR", xData, yData);

    plot.plot(save);
}

void Menu::handleOption5() {
    if (!isDataProcessed) {
        std::cout << "Error: Sector map is empty. Run Option 1 first." << std::endl;
        return;
    }
    all_sectors_summary(sectorMap);   
    if (save == true) {
        std::ofstream outFile("results/Results_Summary.txt", std::ios::app);
        if (outFile) {
            outFile << "\n--- Sector Summary Report ---\n";
            all_sectors_summary(sectorMap, outFile);
            outFile.close();
        }
    }
}

void Menu::clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(1000, '\n');
}

int main() {
    Menu menu;
    menu.run();
    return 0;
}