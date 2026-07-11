#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include "gtfs_loader.h"
#include "graph.h"
#include "pathfinder.h"

using namespace std;

namespace {

void printHeader() {
    cout << "\n" << string(55, '=') << "\n";
    cout << "   PUNE BUS ROUTE PLANNER (PMPML network model, C++)\n";
    cout << string(55, '=') << "\n";
}

void printMenu() {
    cout << "\n1. List all bus stops\n"
         << "2. List all routes\n"
         << "3. Show stops on a route\n"
         << "4. Find shortest path (Dijkstra)\n"
         << "5. Find shortest path (A*)\n"
         << "6. Compare Dijkstra vs A* on the same query\n"
         << "7. Exit\n\n"
         << "Choose an option: ";
}

void listStops(const unordered_map<string, Stop>& stops) {
    // sort numerically by stop_id for readable output
    map<int, string> sortedIds;
    for (const auto& [id, s] : stops) sortedIds[stoi(id)] = id;

    cout << "\n" << left << setw(4) << "ID" << setw(25) << "Stop Name"
         << setw(10) << "Lat" << setw(10) << "Lon" << "\n";
    cout << string(55, '-') << "\n";
    for (const auto& [_, id] : sortedIds) {
        const Stop& s = stops.at(id);
        cout << left << setw(4) << id << setw(25) << s.name
             << setw(10) << s.lat << setw(10) << s.lon << "\n";
    }
}

void listRoutes(const unordered_map<string, Route>& routes) {
    cout << "\n" << left << setw(8) << "Route" << "Name\n";
    cout << string(55, '-') << "\n";
    for (const auto& [id, r] : routes) {
        cout << left << setw(8) << r.shortName << r.longName << "\n";
    }
}

void showRouteStops(const unordered_map<string, Route>& routes,
                     const unordered_map<string, string>& trips,
                     const unordered_map<string, vector<pair<int, string>>>& stopTimes,
                     const unordered_map<string, Stop>& stops) {
    cout << "Enter route number (e.g. 4): ";
    string routeShort;
    getline(cin, routeShort);

    string routeId;
    for (const auto& [id, r] : routes) {
        if (r.shortName == routeShort) { routeId = id; break; }
    }
    if (routeId.empty()) { cout << "Route not found.\n"; return; }

    string tripId;
    for (const auto& [tid, rid] : trips) {
        if (rid == routeId) { tripId = tid; break; }
    }
    if (tripId.empty()) { cout << "No trip data for this route.\n"; return; }

    cout << "\nRoute " << routeShort << ": " << routes.at(routeId).longName << "\n";
    for (const auto& [seq, stopId] : stopTimes.at(tripId)) {
        cout << "  " << seq << ". " << stops.at(stopId).name << "\n";
    }
}

// Prompts for a stop name and resolves it to a stop_id, handling
// ambiguous / not-found cases by re-prompting.
string resolveStop(const unordered_map<string, Stop>& stops, const string& prompt) {
    while (true) {
        cout << prompt;
        string name;
        getline(cin, name);

        vector<string> matches = routegraph::findStopIdsByName(stops, name);
        if (matches.empty()) {
            cout << "  No stop found matching '" << name << "'. Try again.\n";
            continue;
        }
        if (matches.size() == 1) return matches[0];

        cout << "  Multiple matches found:\n";
        for (const auto& id : matches) {
            cout << "    " << id << ": " << stops.at(id).name << "\n";
        }
        cout << "  Enter the exact stop name from the list above: ";
        string exact;
        getline(cin, exact);
        vector<string> exactMatches = routegraph::findStopIdsByName(stops, exact);
        if (exactMatches.size() == 1) return exactMatches[0];
        cout << "  Still ambiguous or not found, please try again.\n";
    }
}

pathfinder::Metric chooseMetric() {
    cout << "Optimize by (1) Time  or (2) Distance? [1/2]: ";
    string choice;
    getline(cin, choice);
    return (choice == "2") ? pathfinder::Metric::DISTANCE : pathfinder::Metric::TIME;
}

void printPathResult(const unordered_map<string, Stop>& stops,
                      const pathfinder::PathResult& result,
                      pathfinder::Metric metric) {
    if (!result.found) {
        cout << "\nNo route found between these stops.\n";
        return;
    }

    string unit = (metric == pathfinder::Metric::DISTANCE) ? "km" : "min";
    string label = (metric == pathfinder::Metric::DISTANCE) ? "distance" : "time";
    cout << "\nTotal " << label << ": " << fixed << setprecision(1) << result.totalCost << " " << unit
         << "   |   Transfers: " << result.transfers
         << "   |   Nodes explored: " << result.nodesExplored << "\n";
    cout << string(55, '-') << "\n";

    string currentRoute = "\0";  // sentinel that won't match a real route id
    bool first = true;
    for (size_t i = 0; i < result.stops.size(); ++i) {
        const string& stopName = stops.at(result.stops[i]).name;
        if (i == 0) {
            cout << "  START  -> " << stopName << "\n";
        } else {
            const Edge& leg = result.legs[i - 1];
            if (leg.routeId != currentRoute) {
                cout << "    [" << (first ? "Board" : "Transfer to") << " Route " << leg.routeName << "]\n";
                currentRoute = leg.routeId;
                first = false;
            }
            string marker = (i == result.stops.size() - 1) ? "END " : "    ";
            cout << "  " << marker << "-> " << stopName << "  ("
                 << fixed << setprecision(1) << leg.timeMin << " min / "
                 << setprecision(2) << leg.distanceKm << " km)\n";
        }
    }
}

void runPathfind(const unordered_map<string, Stop>& stops,
                  const routegraph::Graph& graph,
                  bool useAstar) {
    string start = resolveStop(stops, "From stop: ");
    string end = resolveStop(stops, "To stop: ");
    pathfinder::Metric metric = chooseMetric();

    auto t0 = chrono::steady_clock::now();
    pathfinder::PathResult result = useAstar
        ? pathfinder::astar(graph, stops, start, end, metric)
        : pathfinder::dijkstra(graph, start, end, metric);
    auto t1 = chrono::steady_clock::now();
    double elapsedMs = chrono::duration<double, milli>(t1 - t0).count();

    printPathResult(stops, result, metric);
    if (result.found) {
        cout << "  (" << (useAstar ? "astar" : "dijkstra") << " computed in "
             << fixed << setprecision(3) << elapsedMs << " ms)\n";
    }
}

void runCompare(const unordered_map<string, Stop>& stops, const routegraph::Graph& graph) {
    string start = resolveStop(stops, "From stop: ");
    string end = resolveStop(stops, "To stop: ");
    pathfinder::Metric metric = chooseMetric();

    auto t0 = chrono::steady_clock::now();
    auto dResult = pathfinder::dijkstra(graph, start, end, metric);
    auto t1 = chrono::steady_clock::now();
    double dTime = chrono::duration<double, milli>(t1 - t0).count();

    t0 = chrono::steady_clock::now();
    auto aResult = pathfinder::astar(graph, stops, start, end, metric);
    t1 = chrono::steady_clock::now();
    double aTime = chrono::duration<double, milli>(t1 - t0).count();

    cout << "\n--- Dijkstra ---";
    printPathResult(stops, dResult, metric);
    cout << "  Nodes explored: " << dResult.nodesExplored << ", time: "
         << fixed << setprecision(3) << dTime << " ms\n";

    cout << "\n--- A* ---";
    printPathResult(stops, aResult, metric);
    cout << "  Nodes explored: " << aResult.nodesExplored << ", time: "
         << fixed << setprecision(3) << aTime << " ms\n";

    if (dResult.found && aResult.found) {
        bool sameCost = fabs(dResult.totalCost - aResult.totalCost) < 1e-6;
        cout << "\nBoth algorithms found the same total cost: " << (sameCost ? "YES" : "NO (bug!)") << "\n";
        if (dResult.nodesExplored >= aResult.nodesExplored) {
            cout << "A* explored " << (dResult.nodesExplored - aResult.nodesExplored)
                 << " fewer nodes than Dijkstra.\n";
        } else {
            cout << "A* explored more nodes than Dijkstra on this query.\n";
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    string dataDir = (argc > 1) ? argv[1] : "data";

    cout << "Loading GTFS data from: " << dataDir << "\n";
    unordered_map<string, Stop> stops;
    unordered_map<string, Route> routes;
    unordered_map<string, string> trips;
    unordered_map<string, vector<pair<int, string>>> stopTimes;

    try {
        stops = gtfs::loadStops(dataDir + "/stops.txt");
        routes = gtfs::loadRoutes(dataDir + "/routes.txt");
        trips = gtfs::loadTrips(dataDir + "/trips.txt");
        stopTimes = gtfs::loadStopTimes(dataDir + "/stop_times.txt");
    } catch (const std::exception& e) {
        cerr << "Error loading GTFS data: " << e.what() << "\n";
        return 1;
    }

    routegraph::Graph graph = routegraph::buildGraph(stops, routes, trips, stopTimes);
    cout << "Loaded " << stops.size() << " stops, " << routes.size() << " routes, "
         << trips.size() << " trips.\n";

    printHeader();
    while (true) {
        printMenu();
        string choice;
        if (!getline(cin, choice)) break;

        if (choice == "1") listStops(stops);
        else if (choice == "2") listRoutes(routes);
        else if (choice == "3") showRouteStops(routes, trips, stopTimes, stops);
        else if (choice == "4") runPathfind(stops, graph, false);
        else if (choice == "5") runPathfind(stops, graph, true);
        else if (choice == "6") runCompare(stops, graph);
        else if (choice == "7") { cout << "Goodbye!\n"; break; }
        else cout << "Invalid option, try again.\n";
    }

    return 0;
}
