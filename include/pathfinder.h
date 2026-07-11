#pragma once
#include <string>
#include <vector>
#include "graph.h"
#include "types.h"

namespace pathfinder {

enum class Metric { TIME, DISTANCE };

struct PathResult {
    bool found = false;
    double totalCost = 0.0;
    std::vector<std::string> stops;  // ordered stop_ids from start to end
    std::vector<Edge> legs;          // edge actually taken for each hop
    int transfers = 0;
    int nodesExplored = 0;
};

// Dijkstra's algorithm: O((V + E) log V) with a binary heap.
// Explores nodes in order of increasing cost from `start`; guarantees the
// optimal path with respect to `metric`.
PathResult dijkstra(const routegraph::Graph& graph,
                     const std::string& start,
                     const std::string& end,
                     Metric metric = Metric::TIME);

// A* search: same worst-case complexity as Dijkstra, but explores far
// fewer nodes in practice thanks to a straight-line-distance heuristic
// (haversine distance to the goal, converted to a minimum-possible time
// when optimizing for time). The heuristic never overestimates true
// remaining cost, so it's admissible -- A* still returns the optimal path.
PathResult astar(const routegraph::Graph& graph,
                  const std::unordered_map<std::string, Stop>& stopsData,
                  const std::string& start,
                  const std::string& end,
                  Metric metric = Metric::TIME);

}  // namespace pathfinder
