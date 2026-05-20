#include "../include/common/utils.h"

// Splits CSV line into list of fields
std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    std::istringstream ss(line);
    while (std::getline(ss, field, ','))
        fields.push_back(field);
    return fields;
}

// Group
std::string groupToString(Group g) {
    switch (g) {
        case Group::Beat: return "Beat";
        case Group::Meet: return "Meet";
        case Group::Miss: return "Miss";
        case Group::Unassigned: return "Unassigned";
        default: return "Unknown";
    }
}

// Datetime utilities
bool parseIsoDate(const std::string& date, std::tm& outTm) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    try {
        const int year  = std::stoi(date.substr(0, 4));
        const int month = std::stoi(date.substr(5, 2));
        const int day   = std::stoi(date.substr(8, 2));
        std::tm tm{};
        tm.tm_year = year - 1900;
        tm.tm_mon  = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = 12;
        if (std::mktime(&tm) == -1) return false;
        outTm = tm;
        return true;
    } 
    catch (...) { return false; }
}

std::string formatIsoDate(const std::tm& tmValue) {
    std::ostringstream oss;
    oss << (tmValue.tm_year + 1900) << "-";
    if (tmValue.tm_mon + 1 < 10) oss << "0";
    oss << (tmValue.tm_mon + 1) << "-";
    if (tmValue.tm_mday < 10) oss << "0";
    oss << tmValue.tm_mday;
    return oss.str();
}

bool shiftIsoDate(const std::string& date, int days, std::string& shiftedDate) {
    std::tm tm{};
    if (!parseIsoDate(date, tm)) return false;
    tm.tm_mday += days;
    if (std::mktime(&tm) == -1) return false;
    shiftedDate = formatIsoDate(tm);
    return true;
}

// Warning helpers
void addWarning(std::vector<std::string>& warnings, const std::string& ticker, const std::string& message) {
    warnings.push_back(ticker + ": " + message);
}

std::string joinDateMessage(const std::string& originalDate, const std::string& replacementDate) {
    return "announcement date " + originalDate + " is not a trading day; using next available trading day " + replacementDate;
}