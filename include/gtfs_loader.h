#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "types.h"


namespace gtfs {


std::unordered_map<std::string, Stop> loadStops(const std::string& path);


std::unordered_map<std::string, Route> loadRoutes(const std::string& path);


std::unordered_map<std::string, std::string> loadTrips(const std::string& path);


std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>
loadStopTimes(const std::string& path);

}  
