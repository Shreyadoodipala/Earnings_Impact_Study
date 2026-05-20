#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <chrono>

// Splits CSV line into list of fields
std::vector<std::string> splitCSVLine(const std::string& line);

// Group enum
enum class Group { Beat = 1, Meet = 0, Miss = -1, Unassigned = -2 };
std::string groupToString(Group g);

// Datetime utilities
bool parseIsoDate(const std::string& date, std::tm& outTm);
std::string formatIsoDate(const std::tm& tmValue);
bool shiftIsoDate(const std::string& date, int days, std::string& shiftedDate);

// Warning helpers
void addWarning(std::vector<std::string>& warnings, const std::string& ticker, const std::string& message);
std::string joinDateMessage(const std::string& originalDate, const std::string& replacementDate);