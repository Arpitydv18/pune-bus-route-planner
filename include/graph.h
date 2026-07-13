#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "types.h"


namespace routegraph {

constexpr double kAvgSpeedKmph = 20.0; 
constexpr double kDwellTimeMin = 1.0;  

double haversineKm(double lat1, double lon1, double lat2, double lon2);

using Graph = std::unordered_map<std::string, std::vector<Edge>>;


Graph buildGraph(const std::unordered_map<std::string, Stop>& stops,
                  const std::unordered_map<std::string, Route>& routes,
                  const std::unordered_map<std::string, std::string>& trips,
                  const std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>& stopTimes,
                  double avgSpeedKmph = kAvgSpeedKmph,
                  double dwellTimeMin = kDwellTimeMin,
                  bool bidirectional = true);


// - no match -> empty vector
// - ambiguous partial matches -> vector with more than one entry
std::vector<std::string> findStopIdsByName(const std::unordered_map<std::string, Stop>& stops,
                                            const std::string& name);

}  // namespace routegraph
