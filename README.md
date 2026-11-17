# routefinder

A C++ service that finds places along a route, ranked by the actual detour each one adds — not straight-line distance to the route, but the real extra walking time once you account for rivers, highways, and long blocks.

**Example**: from Penn Station to Union Square, searching "coffee" with a 5-minute max detour, should return something like:

```
1. Blue Bottle (W 15th)   +1.2 min detour   40% along route
2. Joe Coffee (Union Sq)  +0.4 min detour   95% along route
```

## Why this is hard

A place that looks close to your route on a map can be a long walk away once you account for the actual road network. Computing that "real detour" for every candidate place along a route, fast, is the core problem this project solves — using S2 geometry for spatial indexing and a bidirectional Dijkstra trick to compute exact detours for every candidate in two graph searches, instead of one search per candidate.

## Architecture

```
client ──► frontend ──┬──► routing service   (road graph, A*, bidirectional Dijkstra)
                      └──► place shards      (text index + S2 spatial index, sharded by region)
```

The frontend asks the routing service for a route, builds a corridor around it, queries the place shards that corridor overlaps, sends candidates back to the routing service for exact detour costs, ranks the results, and returns the top-k.

## Current status

| Component | Status |
|---|---|
| Build system (CMake + vcpkg manifest: S2, Abseil, gRPC, Protobuf, libosmium, GTest, Benchmark) | Done |
| OSM ingest: places (`places.bin`) — named, categorized points of interest | Done |
| OSM ingest: walkable road graph (`graph.bin`) — nodes + haversine-weighted edges | Done |
| Text search: inverted index + BM25 ranking over place names/tags | Done |
| S2 spatial index for radius queries | Not started |
| Road-network routing (Dijkstra / A*) | Not started |
| Corridor search + bidirectional-Dijkstra exact detour | Not started |
| gRPC API, ranking, benchmarks | Not started |
| Sharded microservices, GKE deployment | Not started |
| Observability (Prometheus), demo map | Not started |

Current data: a BBBike extract of New York City (`data/NewYork.osm.pbf`, ~146MB), yielding 67,807 indexed places and a walk graph of ~2.23M nodes / ~2.66M edges.

## Repo layout

```
proto/        gRPC/protobuf service definitions (not yet added)
src/ingest/   OSM .pbf -> places.bin / graph.bin
src/index/    inverted index + BM25 text search over places.bin
src/routing/  road-graph routing (planned)
src/query/    corridor search + detour ranking (planned)
src/server/   service entry points
bench/        Google Benchmark microbenchmarks (planned)
tests/        GoogleTest unit tests
deploy/       Kubernetes/GKE manifests (planned)
web/          Leaflet demo map (planned)
docs/         design notes
```

## Building

Requires `cmake`, `ninja`, and a C++20 compiler. Dependencies are managed by vcpkg (cloned locally as `./vcpkg`, not committed) in manifest mode via `vcpkg.json`.

```sh
cmake --preset default
cmake --build build
```

The first configure builds all vcpkg dependencies from source, which can take 20+ minutes.

## Running

```sh
# Ingest an OSM extract into places.bin / graph.bin
./build/src/ingest/ingest data/NewYork.osm.pbf

# Search the resulting places index
./build/src/index/search_cli data/places.bin "coffee" 10

# Run unit tests
./build/tests/index_test
```

## Tech stack

C++20, CMake + vcpkg, S2 Geometry, Abseil, gRPC/Protobuf, libosmium (OpenStreetMap parsing), GoogleTest, Google Benchmark, Docker, Kubernetes/GKE, Prometheus, Leaflet.
