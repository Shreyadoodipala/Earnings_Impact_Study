#include "../include/data/StockHistory.h"
#include "../include/common/utils.h"
#include <algorithm>
#include <curl/curl.h>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>
#include <thread>
#include <mutex> 
#include <atomic>

// Memory management for curl response
struct MemoryStruct {
    char* memory;
    size_t size;
};

void* myrealloc(void* ptr, size_t size) {
    if (ptr)
        return realloc(ptr, size);
    else
        return malloc(size);
}


int write_data2(void* ptr, size_t size, size_t nmemb, void* data) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)data;
    mem->memory = (char*)myrealloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}

// Aligns the stock's price history dates with the benchmark's available dates, returning the common dates in sorted order.
std::vector<std::string> alignTradingDates(const Stock& stock, 
    const std::map<std::string, double>& iwvPrices) {
    std::vector<std::string> dates;
    const auto& stockPrices = stock.getPriceHistory();
    dates.reserve(std::min(stockPrices.size(), iwvPrices.size()));
    for (const auto& [date, price] : stockPrices) {
        if (iwvPrices.find(date) != iwvPrices.end()) {
            dates.push_back(date);
        }
    }
    return dates;
}

// Functions for fetching price data from EODHD API
std::string buildEodhdUrl(const std::string& ticker, const std::string& startDate, 
    const std::string& endDate, const std::string& apiToken) {
    std::ostringstream url;
    url << "https://eodhistoricaldata.com/api/eod/" << ticker << "?from=" << startDate
        << "&to=" << endDate << "&api_token=" << apiToken << "&period=d";
    return url.str();
}

    
bool fetchFromUrlWithHandle(CURL* curl, const std::string& url, std::string& response, std::string& error) {
    struct MemoryStruct data;
    data.memory = NULL;
    data.size   = 0;
   
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data2);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode status = curl_easy_perform(curl);

    if (status != CURLE_OK) {
        free(data.memory);
        error = curl_easy_strerror(status);
        return false;
    }

    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    if (responseCode != 200) {
        free(data.memory);
        std::ostringstream oss;
        oss << "HTTP " << responseCode;
        error = oss.str();
        return false;
    }

    if (!data.memory || data.size == 0) {
        free(data.memory);
        error = "empty response";
        return false;
    }

    response = std::string(data.memory, data.size);
    free(data.memory);
    return true;
}

bool fetchFromUrl(const std::string& url, std::string& response, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        error = "failed to initialize curl";
        return false;
    }
    bool ok = fetchFromUrlWithHandle(curl, url, response, error);
    curl_easy_cleanup(curl);
    return ok;
}

std::string toEodhdUsTicker(const std::string& ticker) {
    if (ticker.find('.') != std::string::npos) return ticker;
    return ticker + ".US";
}

// Parse EODHD response
bool parseEodhdCsvToPriceMap(const std::string& csv, std::map<std::string, double>& prices, 
    std::string& error) {
    std::istringstream stream(csv);
    std::string line;

    if (!std::getline(stream, line)) {
        error = "response missing CSV header";
        return false;
    }

    prices.clear();
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        std::vector<std::string> fields = splitCSVLine(line);
        if (fields.size() < 6) continue;

        const std::string& date = fields[0];
        double closePrice = std::numeric_limits<double>::quiet_NaN();
        try { closePrice = std::stod(fields[5]); } // Adjusted_close
        catch (...) {
            try { closePrice = std::stod(fields[4]); } // Close fallback 
            catch (...) { continue; }
        }
        prices[date] = closePrice;
    }

    if (prices.empty()) {
        error = "no price rows parsed from CSV";
        return false;
    }
    return true;
}

// Date range to fetch benchmark price history
bool fetchDateRangeFromAnnouncements(const std::map<std::string, Stock>& stocks,
    int N, std::string& startDate, std::string& endDate, std::string& error) {
    if (stocks.empty()) { error = "stocks map is empty"; return false; }
    if (N < 0) { error = "N must be non-negative"; return false; }

    std::string minAnnouncement = "9999-12-31";
    std::string maxAnnouncement = "0000-01-01";

    for (const auto& [ticker, stock] : stocks) {
        const std::string day0 = stock.getAnnouncementDate();
        std::tm parsed{};
        if (!parseIsoDate(day0, parsed)) {
            error = "invalid announcement date for " + ticker + ": " + day0;
            return false;
        }
        if (day0 < minAnnouncement) minAnnouncement = day0;
        if (day0 > maxAnnouncement) maxAnnouncement = day0;
    }

    const int paddingDays = std::max(30, N + 5);
    if (!shiftIsoDate(minAnnouncement, -paddingDays, startDate)) {
        error = "failed to compute start date from " + minAnnouncement;
        return false;
    }
    if (!shiftIsoDate(maxAnnouncement, paddingDays, endDate)) {
        error = "failed to compute end date from " + maxAnnouncement;
        return false;
    }
    return true;
}

// Main functions
PreparedEventWindow::PreparedEventWindow(): eventIndex(-1), eventWindowIndex(-1), valid(false) {}

void PreparedEventWindow::setBenchmarkPrices(const Vector& prices) { benchmarkPrices = prices; }
void PreparedEventWindow::setBenchmarkPrices(Vector&& prices) { benchmarkPrices = std::move(prices); }

int findEventDayIndex(const Stock& stock, std::vector<std::string>& warnings) {
    const std::string announcementDate = stock.getAnnouncementDate();
    const std::vector<std::string> stockDates = stock.getPriceDates();

    if (stockDates.empty()) {
        addWarning(warnings, stock.getTicker(), "has no stock price history");
        return -1;
    }

    auto it = std::lower_bound(stockDates.begin(), stockDates.end(), announcementDate);
    if (it == stockDates.end()) {
        addWarning(warnings, stock.getTicker(),
            "announcement date " + announcementDate + " is after the last available stock trading day");
        return -1;
    }

    if (*it != announcementDate) {
        addWarning(warnings, stock.getTicker(), joinDateMessage(announcementDate, *it));
    }

    return static_cast<int>(std::distance(stockDates.begin(), it));
}

PreparedEventWindow prepareEventWindow(const Stock& stock, 
    const std::map<std::string, double>& iwvPrices, int N) {
    PreparedEventWindow result;
    result.ticker           = stock.getTicker();
    result.announcementDate = stock.getAnnouncementDate();

    if (N < 0) {
        addWarning(result.warnings, stock.getTicker(), "N must be non-negative");
        return result;
    }

    const int requiredWindowSize = 2 * N + 1;
    const auto& stockPrices = stock.getPriceHistory();

    if (stockPrices.size() < static_cast<size_t>(requiredWindowSize)) {
        std::ostringstream msg;
        msg << "has only " << stockPrices.size()
            << " stock price rows; requires at least " << requiredWindowSize;
        addWarning(result.warnings, stock.getTicker(), msg.str());
        return result;
    }

    if (iwvPrices.size() < static_cast<size_t>(requiredWindowSize)) {
        std::ostringstream msg;
        msg << "IWV benchmark has only " << iwvPrices.size()
            << " price rows; requires at least " << requiredWindowSize;
        addWarning(result.warnings, stock.getTicker(), msg.str());
        return result;
    }

    std::vector<std::string> eventWarnings;
    const int stockEventIndex = findEventDayIndex(stock, eventWarnings);
    result.warnings.insert(result.warnings.end(), eventWarnings.begin(), eventWarnings.end());
    if (stockEventIndex < 0) return result;

    const std::vector<std::string> stockDates    = stock.getPriceDates();
    const std::string              stockEventDate = stockDates[stockEventIndex];
    std::vector<std::string>       alignedDates   = alignTradingDates(stock, iwvPrices);

    if (alignedDates.size() < static_cast<size_t>(requiredWindowSize)) {
        std::ostringstream msg;
        msg << "has only " << alignedDates.size()
            << " stock/IWV aligned trading days; requires at least " << requiredWindowSize;
        addWarning(result.warnings, stock.getTicker(), msg.str());
        return result;
    }

    auto eventIt = std::lower_bound(alignedDates.begin(), alignedDates.end(), stockEventDate);
    if (eventIt == alignedDates.end()) {
        addWarning(result.warnings, stock.getTicker(),
            "event trading day " + stockEventDate + " has no matching or later IWV benchmark date");
        return result;
    }

    if (*eventIt != stockEventDate) {
        addWarning(result.warnings, stock.getTicker(),
            "event trading day " + stockEventDate +
            " is missing from aligned IWV data; using next aligned trading day " + *eventIt);
    }

    const int alignedEventIndex = static_cast<int>(std::distance(alignedDates.begin(), eventIt));
    const int daysBefore = alignedEventIndex;
    const int daysAfter  = static_cast<int>(alignedDates.size()) - alignedEventIndex - 1;

    if (daysBefore < N || daysAfter < N) {
        std::ostringstream msg;
        msg << "insufficient aligned trading days around event date " << *eventIt
            << "; has " << daysBefore << " before and " << daysAfter
            << " after, requires " << N << " each";
        addWarning(result.warnings, stock.getTicker(), msg.str());
        return result;
    }

    const int start = alignedEventIndex - N;
    const int end   = alignedEventIndex + N;
    result.eventDate        = *eventIt;
    result.eventIndex       = alignedEventIndex;
    result.eventWindowIndex = N;
    result.dates.reserve(requiredWindowSize);
    result.stockPrices.reserve(requiredWindowSize);

    Vector benchmarkWindow;
    benchmarkWindow.reserve(requiredWindowSize);

    for (int i = start; i <= end; ++i) {
        const std::string& date = alignedDates[i];
        result.dates.push_back(date);
        result.stockPrices.push_back(stock.getPrice(date));
        benchmarkWindow.push_back(iwvPrices.at(date));
    }

    result.setBenchmarkPrices(std::move(benchmarkWindow));
    result.valid = true;
    return result;
}

StockPrepSummary prepareEventWindows(const std::map<std::string, Stock>& stocks,
    const std::map<std::string, double>& iwvPrices, int N) {
    StockPrepSummary summary;
    for (const auto& [ticker, stock] : stocks) {
        PreparedEventWindow window = prepareEventWindow(stock, iwvPrices, N);
        if (window.valid)
            summary.validWindows.emplace(ticker, window);
        else
            summary.skippedStocks.emplace(ticker, window.warnings);
    }
    return summary;
}

std::map<std::string, Stock> stocksForValidWindows(const std::map<std::string, Stock>& universe,
    const StockPrepSummary& summary) {
    std::map<std::string, Stock> out;
    for (const auto& kv : summary.validWindows) {
        auto it = universe.find(kv.first);
        if (it != universe.end())
            out.emplace(it->first, it->second);
    }
    return out;
}

std::vector<std::string> populatePriceHistoryFromEODHD(std::map<std::string, Stock>& stocks,
    std::map<std::string, double>& benchmarkPrices, const std::string& benchmarkTicker,
    int N, const std::string& apiToken) {
    std::vector<std::string> warnings;

    if (apiToken.empty()) {
        warnings.push_back("EODHD API token is empty.");
        return warnings;
    }

    std::string startDate, endDate, rangeError;
    if (!fetchDateRangeFromAnnouncements(stocks, N, startDate, endDate, rangeError)) {
        warnings.push_back("Could not derive EODHD date range: " + rangeError);
        return warnings;
    }
   
    curl_global_init(CURL_GLOBAL_ALL);

    const std::string benchmarkSymbol = toEodhdUsTicker(benchmarkTicker);
    const std::string benchmarkUrl    = buildEodhdUrl(benchmarkSymbol, startDate, endDate, apiToken);
    std::string benchmarkResponse, benchmarkError;

    std::cout << "Fetching benchmark " << benchmarkSymbol << "..." << std::endl;
    if (!fetchFromUrl(benchmarkUrl, benchmarkResponse, benchmarkError) ||
        !parseEodhdCsvToPriceMap(benchmarkResponse, benchmarkPrices, benchmarkError)) {
        warnings.push_back("Failed to fetch benchmark " + benchmarkSymbol + ": " + benchmarkError);
    } else {
        std::cout << "Benchmark fetched: " << benchmarkPrices.size() << " trading days." << std::endl;
    }

    int total = (int)stocks.size();
    ProgressBar progress(total);

    std::cout << "Fetching " << total << " stocks from eodhistoricaldata.com..." << std::endl;
    progress.show(0);

    std::vector<std::pair<std::string, Stock*>> stockList;
    for (auto& [ticker, stock] : stocks)
        stockList.push_back({ticker, &stock});

    std::mutex mtx;
    std::atomic<int> counter(0);

    const int NUM_THREADS = 3; 
    int chunkSize = ((int)stockList.size() + NUM_THREADS - 1) / NUM_THREADS;

    auto downloadChunk = [&](int start, int end) {
        CURL* handle = curl_easy_init();  
        if (!handle) return;

        for (int i = start; i < end && i < (int)stockList.size(); i++) {
            auto& [ticker, stockPtr] = stockList[i];
            const std::string symbol   = toEodhdUsTicker(ticker);
            const std::string stockUrl = buildEodhdUrl(symbol, startDate, endDate, apiToken);

            std::string response, error;
            std::map<std::string, double> stockPrices;

            if (!fetchFromUrlWithHandle(handle, stockUrl, response, error) ||
                !parseEodhdCsvToPriceMap(response, stockPrices, error)) {
                std::lock_guard<std::mutex> lock(mtx);
                warnings.push_back("Failed to fetch " + symbol + ": " + error);
            } else {
                
                for (const auto& [date, price] : stockPrices)
                    stockPtr->addPrice(date, price);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            int done = ++counter;
            std::lock_guard<std::mutex> lock(mtx);
            progress.show(done);
        }
        curl_easy_cleanup(handle);
    };

    // use threads
    std::vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; t++) {
        int start = t * chunkSize;
        int end   = start + chunkSize;
        threads.emplace_back(downloadChunk, start, end);
    }
    for (auto& t : threads)
        t.join();

    std::cout << std::endl << "Download complete!" << std::endl;

    curl_global_cleanup();
    return warnings;
}