#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "types.h"

// Loads GTFS-style CSV files into in-memory lookup tables.
//
// Works with the small sample dataset shipped in data/, and should also
// work with a real PMPML GTFS feed downloaded from the Pune Open Data
// Portal (opendata.punecorporation.org) -- real feeds have extra columns
// (arrival_time, shape_id, wheelchair_boarding, ...) which this loader
// simply ignores since it looks columns up by header name.
namespace gtfs {

// stop_id -> Stop
std::unordered_map<std::string, Stop> loadStops(const std::string& path);

// route_id -> Route
std::unordered_map<std::string, Route> loadRoutes(const std::string& path);

// trip_id -> route_id
std::unordered_map<std::string, std::string> loadTrips(const std::string& path);

// trip_id -> ordered list of (stop_sequence, stop_id), sorted by sequence
std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>
loadStopTimes(const std::string& path);

}  // namespace gtfs
