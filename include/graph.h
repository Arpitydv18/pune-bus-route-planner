#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "types.h"

// Builds and queries the weighted multigraph of bus stops.
namespace routegraph {

constexpr double kAvgSpeedKmph = 20.0;  // average city bus speed incl. traffic
constexpr double kDwellTimeMin = 1.0;   // avg time lost per stop (boarding/lights)

// Great-circle distance between two lat/lon points, in kilometers.
double haversineKm(double lat1, double lon1, double lat2, double lon2);

using Graph = std::unordered_map<std::string, std::vector<Edge>>;

// Builds the graph: stop_id -> list of outgoing Edges.
// If `bidirectional` is true (default), each hop is added in both
// directions, which approximates most real bus routes running the same
// road both ways. Set to false to strictly respect trip direction only
// (recommended once you plug in a real GTFS feed with separate up/down trips).
Graph buildGraph(const std::unordered_map<std::string, Stop>& stops,
                  const std::unordered_map<std::string, Route>& routes,
                  const std::unordered_map<std::string, std::string>& trips,
                  const std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>& stopTimes,
                  double avgSpeedKmph = kAvgSpeedKmph,
                  double dwellTimeMin = kDwellTimeMin,
                  bool bidirectional = true);

// Case-insensitive stop name lookup. Returns matching stop_id(s):
// - exactly one match -> that vector has size 1
// - no match -> empty vector
// - ambiguous partial matches -> vector with more than one entry
std::vector<std::string> findStopIdsByName(const std::unordered_map<std::string, Stop>& stops,
                                            const std::string& name);

}  // namespace routegraph
