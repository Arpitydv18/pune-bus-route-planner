#pragma once
#include <string>


struct Stop {
    std::string id;
    std::string name;
    double lat = 0.0;
    double lon = 0.0;
};


struct Route {
    std::string id;
    std::string shortName;
    std::string longName;
};



struct Edge {
    std::string toStop;
    std::string routeId;
    std::string routeName;
    double distanceKm = 0.0;
    double timeMin = 0.0;
};
