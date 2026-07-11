#include "gtfs_loader.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace gtfs {

namespace {

// Splits a CSV line on commas. The sample/GTFS data used here has no
// quoted fields or embedded commas, so a simple split is sufficient.
std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

// Reads a CSV file into a header-indexed list of rows:
// each row is a map from column name -> value.
std::vector<std::unordered_map<std::string, std::string>> readCsv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::vector<std::unordered_map<std::string, std::string>> rows;
    std::string headerLine;
    if (!std::getline(file, headerLine)) {
        return rows;  // empty file
    }
    // strip potential trailing \r (Windows line endings)
    if (!headerLine.empty() && headerLine.back() == '\r') headerLine.pop_back();
    std::vector<std::string> headers = splitCsvLine(headerLine);

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::vector<std::string> fields = splitCsvLine(line);

        std::unordered_map<std::string, std::string> row;
        for (size_t i = 0; i < headers.size() && i < fields.size(); ++i) {
            row[headers[i]] = fields[i];
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

}  // namespace

std::unordered_map<std::string, Stop> loadStops(const std::string& path) {
    std::unordered_map<std::string, Stop> stops;
    for (const auto& row : readCsv(path)) {
        Stop s;
        s.id = row.at("stop_id");
        s.name = row.at("stop_name");
        s.lat = std::stod(row.at("stop_lat"));
        s.lon = std::stod(row.at("stop_lon"));
        stops[s.id] = s;
    }
    return stops;
}

std::unordered_map<std::string, Route> loadRoutes(const std::string& path) {
    std::unordered_map<std::string, Route> routes;
    for (const auto& row : readCsv(path)) {
        Route r;
        r.id = row.at("route_id");
        r.shortName = row.at("route_short_name");
        auto it = row.find("route_long_name");
        r.longName = (it != row.end()) ? it->second : "";
        routes[r.id] = r;
    }
    return routes;
}

std::unordered_map<std::string, std::string> loadTrips(const std::string& path) {
    std::unordered_map<std::string, std::string> trips;
    for (const auto& row : readCsv(path)) {
        trips[row.at("trip_id")] = row.at("route_id");
    }
    return trips;
}

std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>
loadStopTimes(const std::string& path) {
    std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> stopTimes;
    for (const auto& row : readCsv(path)) {
        const std::string& tripId = row.at("trip_id");
        int seq = std::stoi(row.at("stop_sequence"));
        stopTimes[tripId].emplace_back(seq, row.at("stop_id"));
    }
    for (auto& [tripId, seq] : stopTimes) {
        std::sort(seq.begin(), seq.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
    }
    return stopTimes;
}

}  // namespace gtfs
