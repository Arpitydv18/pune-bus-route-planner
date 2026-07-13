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
    std::vector<std::string> stops;  
    std::vector<Edge> legs;          
    int transfers = 0;
    int nodesExplored = 0;
};


PathResult dijkstra(const routegraph::Graph& graph,
                     const std::string& start,
                     const std::string& end,
                     Metric metric = Metric::TIME);


PathResult astar(const routegraph::Graph& graph,
                  const std::unordered_map<std::string, Stop>& stopsData,
                  const std::string& start,
                  const std::string& end,
                  Metric metric = Metric::TIME);

}  
