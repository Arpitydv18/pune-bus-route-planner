# Pune Bus Route Planner (C++)

A C++17 shortest-path route planner for Pune's PMPML bus network, built on
GTFS-style transit data. Given two stops, it finds the fastest (or
shortest) route across the network — including transfers between routes —
using **Dijkstra's algorithm** and **A\* search**.

This is a C++ port of the original Python prototype, with identical
behavior and output format, for use in projects/resumes where a C++
implementation is expected.

## Building

### In VS Code (recommended if you're using it)

This project includes a `.vscode/` folder with build and debug configs already set up:

1. Open the project folder in VS Code (`File → Open Folder`)
2. Install the **C/C++ Extension Pack** (by Microsoft) if you haven't already
3. Press **Ctrl+Shift+B** (Cmd+Shift+B on Mac) to build — this runs the "Build route_planner (debug)" task
4. Run it from the integrated terminal: `./route_planner`
5. To debug: open `src/pathfinder.cpp`, click in the gutter to set a breakpoint (e.g. inside the Dijkstra loop), then press **F5** — it will build automatically and stop at your breakpoint

Notes:
- `launch.json` assumes `gdb` at `/usr/bin/gdb` (standard on Linux, and available via WSL on Windows). On macOS, install Xcode Command Line Tools and switch `MIMode` to `lldb` in `.vscode/launch.json`, or install gdb via Homebrew.
- On Windows without WSL, install MinGW-w64 and point `miDebuggerPath` at its `gdb.exe`, or use the MSVC-based `cppvsdbg` debugger type instead.
- Because this is an interactive console app (it reads menu choices from stdin), debugging works fine in the integrated terminal — just type your menu choices as normal once it hits a breakpoint or runs to completion.

### With g++ directly (no dependencies beyond a C++17 compiler)

```bash
g++ -std=c++17 -O2 -Iinclude -o route_planner src/main.cpp src/gtfs_loader.cpp src/graph.cpp src/pathfinder.cpp
./route_planner
```

### With CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
./route_planner        # data/ path is relative to wherever you run it from
```

If you build out-of-source with CMake, pass the data directory explicitly:

```bash
./build/route_planner ../data
```

## Project structure

```
pune-bus-route-planner-cpp/
├── CMakeLists.txt
├── data/                    # sample GTFS-style dataset (see below)
│   ├── stops.txt
│   ├── routes.txt
│   ├── trips.txt
│   └── stop_times.txt
├── include/
│   ├── types.h               # Stop / Route / Edge structs
│   ├── gtfs_loader.h
│   ├── graph.h
│   └── pathfinder.h
└── src/
    ├── gtfs_loader.cpp        # CSV -> in-memory tables
    ├── graph.cpp              # builds the weighted stop multigraph
    ├── pathfinder.cpp         # Dijkstra + A*
    └── main.cpp               # interactive CLI menu
```

## Usage

```
1. List all bus stops
2. List all routes
3. Show stops on a route
4. Find shortest path (Dijkstra)
5. Find shortest path (A*)
6. Compare Dijkstra vs A* on the same query
7. Exit
```

## About the sample data

⚠️ The dataset in `data/` is a small, hand-built sample — 25 real Pune
localities (Swargate, Kothrud, Hinjewadi, Hadapsar, etc.) with approximately
correct coordinates, connected by 8 representative routes. It's realistic
enough to demonstrate transfers and multiple paths, but it is **not** the
actual PMPML network.

To use real data: download the PMPML GTFS feed from Pune's Open Data Portal
(`opendata.punecorporation.org`), extract it, and run:
```bash
./route_planner /path/to/extracted/gtfs
```
`gtfs_loader.cpp` reads columns by header name and ignores extras, so real
GTFS files (which have more columns than the sample) should load without
changes.

## Algorithm details

**Graph model:** stops are nodes; each consecutive pair of stops on a
route's path is an edge (`Edge{toStop, routeId, routeName, distanceKm, timeMin}`).
The same pair of stops can be connected by edges from multiple routes, so
`routegraph::Graph` (an `unordered_map<string, vector<Edge>>`) is a
multigraph — the pathfinder picks whichever edge is cheapest at each hop.

**Edge weights**
- `timeMin` = (haversine distance / average speed) + dwell time
- `distanceKm` = haversine distance between the two stops

**Dijkstra** (`pathfinder::dijkstra`) — `O((V + E) log V)` using
`std::priority_queue` as a binary min-heap. Explores nodes in order of
increasing cost from the source; guarantees the optimal path.

**A\*** (`pathfinder::astar`) — same worst-case complexity, but explores
far fewer nodes in practice. The heuristic is the straight-line (haversine)
distance to the destination, converted to a minimum-possible time when
optimizing for time. This heuristic never overestimates true remaining
cost, so it's admissible — A* is still guaranteed to find the optimal path.
Use menu option 6 to verify both algorithms return the same total cost
while A* visits fewer nodes.

**Note on tie-breaking:** when two different routes offer an equal-cost
edge between the same pair of stops (e.g. two routes both passing through
Swargate and PMC with identical distance), Dijkstra/A* may pick either one
depending on hash-map iteration order — this does not affect the total
optimal cost, only which of several equally-good paths is reported.

## Possible extensions

- Real-time bus positions/delays (would need a GTFS-realtime feed)
- A fixed transfer penalty in `pathfinder.cpp` to prefer fewer-transfer routes
- Export the resulting path to GeoJSON for map visualization
- Unit tests (e.g. Catch2/GoogleTest) for `graph.cpp` and `pathfinder.cpp`
  against a small fixed test graph with known shortest paths
- A small HTTP API (e.g. with cpp-httplib) instead of a CLI

## Data source

- Pune Open Data Portal — PMPML bus routes GTFS feed:
  http://opendata.punecorporation.org
