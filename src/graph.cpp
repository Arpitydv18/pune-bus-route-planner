#include "graph.h"

#include <algorithm>
#include <cmath>
#include <cctype>

namespace routegraph {

namespace {
constexpr double kPi = 3.14159265358979323846;
}

double haversineKm(double lat1, double lon1, double lat2, double lon2) {
    constexpr double R = 6371.0;  // Earth radius in km
    double phi1 = lat1 * kPi / 180.0;
    double phi2 = lat2 * kPi / 180.0;
    double dphi = (lat2 - lat1) * kPi / 180.0;
    double dlambda = (lon2 - lon1) * kPi / 180.0;

    double a = std::sin(dphi / 2) * std::sin(dphi / 2) +
               std::cos(phi1) * std::cos(phi2) * std::sin(dlambda / 2) * std::sin(dlambda / 2);
    double c = 2 * std::asin(std::sqrt(a));
    return R * c;
}

Graph buildGraph(const std::unordered_map<std::string, Stop>& stops,
                  const std::unordered_map<std::string, Route>& routes,
                  const std::unordered_map<std::string, std::string>& trips,
                  const std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>& stopTimes,
                  double avgSpeedKmph,
                  double dwellTimeMin,
                  bool bidirectional) {
    Graph graph;

    for (const auto& [tripId, sequence] : stopTimes) {
        auto tripIt = trips.find(tripId);
        std::string routeId = (tripIt != trips.end()) ? tripIt->second : "";
        std::string routeName = routeId;
        auto routeIt = routes.find(routeId);
        if (routeIt != routes.end()) routeName = routeIt->second.shortName;

        for (size_t i = 0; i + 1 < sequence.size(); ++i) {
            const std::string& fromId = sequence[i].second;
            const std::string& toId = sequence[i + 1].second;

            auto fromStopIt = stops.find(fromId);
            auto toStopIt = stops.find(toId);
            if (fromStopIt == stops.end() || toStopIt == stops.end()) continue;  // skip malformed rows

            const Stop& a = fromStopIt->second;
            const Stop& b = toStopIt->second;
            double dist = haversineKm(a.lat, a.lon, b.lat, b.lon);
            double timeMin = (dist / avgSpeedKmph) * 60.0 + dwellTimeMin;

            graph[fromId].push_back(Edge{toId, routeId, routeName, dist, timeMin});
            if (bidirectional) {
                graph[toId].push_back(Edge{fromId, routeId, routeName, dist, timeMin});
            }
        }
    }
    return graph;
}

namespace {
std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return out;
}
}  // namespace

std::vector<std::string> findStopIdsByName(const std::unordered_map<std::string, Stop>& stops,
                                            const std::string& name) {
    std::string needle = toLower(name);
    // trim whitespace
    size_t start = needle.find_first_not_of(" \t\n\r");
    size_t end = needle.find_last_not_of(" \t\n\r");
    needle = (start == std::string::npos) ? "" : needle.substr(start, end - start + 1);

    // exact match first
    for (const auto& [id, stop] : stops) {
        if (toLower(stop.name) == needle) return {id};
    }
    // partial match fallback
    std::vector<std::string> matches;
    for (const auto& [id, stop] : stops) {
        if (toLower(stop.name).find(needle) != std::string::npos) {
            matches.push_back(id);
        }
    }
    return matches;
}

}  // namespace routegraph
