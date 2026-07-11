#include "pathfinder.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_set>

namespace pathfinder {

namespace {

double edgeWeight(const Edge& e, Metric metric) {
    return metric == Metric::TIME ? e.timeMin : e.distanceKm;
}

int countTransfers(const std::vector<Edge>& legs) {
    if (legs.empty()) return 0;
    int transfers = 0;
    for (size_t i = 1; i < legs.size(); ++i) {
        if (legs[i].routeId != legs[i - 1].routeId) ++transfers;
    }
    return transfers;
}

PathResult reconstruct(const std::unordered_map<std::string, std::string>& cameFrom,
                        const std::unordered_map<std::string, Edge>& edgeUsed,
                        const std::unordered_map<std::string, double>& costSoFar,
                        const std::string& start,
                        const std::string& end,
                        int nodesExplored) {
    PathResult result;
    result.found = true;
    result.totalCost = costSoFar.at(end);
    result.nodesExplored = nodesExplored;

    std::vector<std::string> stops{end};
    std::vector<Edge> legs;
    std::string cur = end;
    while (cur != start) {
        const Edge& e = edgeUsed.at(cur);
        legs.push_back(e);
        cur = cameFrom.at(cur);
        stops.push_back(cur);
    }
    std::reverse(stops.begin(), stops.end());
    std::reverse(legs.begin(), legs.end());

    result.stops = std::move(stops);
    result.legs = std::move(legs);
    result.transfers = countTransfers(result.legs);
    return result;
}

}  // namespace

PathResult dijkstra(const routegraph::Graph& graph,
                     const std::string& start,
                     const std::string& end,
                     Metric metric) {
    if (graph.find(start) == graph.end() || graph.find(end) == graph.end()) {
        return PathResult{};  // not found
    }

    using QueueItem = std::pair<double, std::string>;  // (cost, node)
    auto cmp = [](const QueueItem& a, const QueueItem& b) { return a.first > b.first; };
    std::priority_queue<QueueItem, std::vector<QueueItem>, decltype(cmp)> pq(cmp);

    std::unordered_map<std::string, double> costSoFar{{start, 0.0}};
    std::unordered_map<std::string, std::string> cameFrom;
    std::unordered_map<std::string, Edge> edgeUsed;
    std::unordered_set<std::string> visited;

    pq.push({0.0, start});
    int nodesExplored = 0;

    while (!pq.empty()) {
        auto [cost, node] = pq.top();
        pq.pop();
        if (visited.count(node)) continue;
        visited.insert(node);
        ++nodesExplored;

        if (node == end) break;

        auto it = graph.find(node);
        if (it == graph.end()) continue;

        for (const Edge& e : it->second) {
            double w = edgeWeight(e, metric);
            double newCost = cost + w;
            auto existing = costSoFar.find(e.toStop);
            if (existing == costSoFar.end() || newCost < existing->second) {
                costSoFar[e.toStop] = newCost;
                cameFrom[e.toStop] = node;
                edgeUsed[e.toStop] = e;
                pq.push({newCost, e.toStop});
            }
        }
    }

    if (costSoFar.find(end) == costSoFar.end()) {
        return PathResult{};  // no path
    }
    return reconstruct(cameFrom, edgeUsed, costSoFar, start, end, nodesExplored);
}

PathResult astar(const routegraph::Graph& graph,
                  const std::unordered_map<std::string, Stop>& stopsData,
                  const std::string& start,
                  const std::string& end,
                  Metric metric) {
    if (graph.find(start) == graph.end() || graph.find(end) == graph.end()) {
        return PathResult{};
    }

    const Stop& goal = stopsData.at(end);
    auto heuristic = [&](const std::string& nodeId) -> double {
        const Stop& s = stopsData.at(nodeId);
        double straightKm = routegraph::haversineKm(s.lat, s.lon, goal.lat, goal.lon);
        if (metric == Metric::DISTANCE) return straightKm;
        return (straightKm / routegraph::kAvgSpeedKmph) * 60.0;  // min possible time
    };

    using QueueItem = std::tuple<double, double, std::string>;  // (f, g, node)
    auto cmp = [](const QueueItem& a, const QueueItem& b) { return std::get<0>(a) > std::get<0>(b); };
    std::priority_queue<QueueItem, std::vector<QueueItem>, decltype(cmp)> pq(cmp);

    std::unordered_map<std::string, double> gScore{{start, 0.0}};
    std::unordered_map<std::string, std::string> cameFrom;
    std::unordered_map<std::string, Edge> edgeUsed;
    std::unordered_set<std::string> visited;

    pq.push({heuristic(start), 0.0, start});
    int nodesExplored = 0;

    while (!pq.empty()) {
        auto [f, g, node] = pq.top();
        pq.pop();
        if (visited.count(node)) continue;
        visited.insert(node);
        ++nodesExplored;

        if (node == end) break;

        auto it = graph.find(node);
        if (it == graph.end()) continue;

        for (const Edge& e : it->second) {
            double w = edgeWeight(e, metric);
            double tentativeG = g + w;
            auto existing = gScore.find(e.toStop);
            if (existing == gScore.end() || tentativeG < existing->second) {
                gScore[e.toStop] = tentativeG;
                cameFrom[e.toStop] = node;
                edgeUsed[e.toStop] = e;
                pq.push({tentativeG + heuristic(e.toStop), tentativeG, e.toStop});
            }
        }
    }

    if (gScore.find(end) == gScore.end()) {
        return PathResult{};
    }
    return reconstruct(cameFrom, edgeUsed, gScore, start, end, nodesExplored);
}

}  // namespace pathfinder
