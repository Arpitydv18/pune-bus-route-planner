#pragma once
#include <string>

// A single bus stop.
struct Stop {
    std::string id;
    std::string name;
    double lat = 0.0;
    double lon = 0.0;
};

// A bus route (e.g. "Route 4: Swargate - Katraj").
struct Route {
    std::string id;
    std::string shortName;
    std::string longName;
};

// A direct hop from one stop to the next stop on some route.
// Edges are stored per-route because the same pair of stops can be
// connected by more than one route (this is a multigraph).
struct Edge {
    std::string toStop;
    std::string routeId;
    std::string routeName;
    double distanceKm = 0.0;
    double timeMin = 0.0;
};
