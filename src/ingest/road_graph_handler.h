#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

#include <osmium/geom/haversine.hpp>
#include <osmium/handler.hpp>
#include <osmium/osm/way.hpp>

#include "src/ingest/graph.h"

constexpr double kWalkingSpeedMps = 1.4;

namespace detail {

// Excludes car-only roads and explicitly non-pedestrian ways, mirroring the
// tag conventions OSM uses for walkability (foot=no/access=private override
// highway class; foot=yes overrides a car-only highway class back in).
inline bool IsWalkable(const osmium::TagList& tags) {
  const char* foot = tags["foot"];
  if (foot != nullptr && std::strcmp(foot, "no") == 0) return false;
  const bool foot_yes = foot != nullptr && std::strcmp(foot, "yes") == 0;

  const char* access = tags["access"];
  if (!foot_yes && access != nullptr &&
      (std::strcmp(access, "private") == 0 || std::strcmp(access, "no") == 0)) {
    return false;
  }

  static constexpr std::array<const char*, 4> kCarOnlyHighways = {
      "motorway", "motorway_link", "trunk", "trunk_link"};
  const char* highway = tags["highway"];
  for (const char* bad : kCarOnlyHighways) {
    if (std::strcmp(highway, bad) == 0) return foot_yes;
  }
  return true;
}

}  // namespace detail

// Must run as the handler *after* osmium::handler::NodeLocationsForWays in an
// osmium::apply(...) call, so each way's node refs already carry resolved
// locations by the time way() runs here.
class RoadGraphHandler : public osmium::handler::Handler {
 public:
  std::vector<GraphNode> nodes;
  std::vector<GraphEdge> edges;

  void way(const osmium::Way& w) {
    const osmium::TagList& tags = w.tags();
    if (tags["highway"] == nullptr) return;
    if (!detail::IsWalkable(tags)) return;

    uint32_t prev_index = kNoIndex;
    osmium::Location prev_location;
    for (const osmium::NodeRef& nr : w.nodes()) {
      if (!nr.location().valid()) {
        // Extract boundary cuts through this way; skip the gap instead of
        // connecting across unknown distance.
        prev_index = kNoIndex;
        continue;
      }

      const uint32_t index = IndexFor(nr);
      if (prev_index != kNoIndex) {
        const double meters =
            osmium::geom::haversine::distance(prev_location, nr.location());
        edges.push_back({prev_index, index, meters / kWalkingSpeedMps});
      }
      prev_index = index;
      prev_location = nr.location();
    }
  }

 private:
  static constexpr uint32_t kNoIndex = static_cast<uint32_t>(-1);
  std::unordered_map<int64_t, uint32_t> osm_id_to_index_;

  uint32_t IndexFor(const osmium::NodeRef& nr) {
    const auto [it, inserted] = osm_id_to_index_.try_emplace(
        nr.ref(), static_cast<uint32_t>(nodes.size()));
    if (inserted) {
      nodes.push_back({nr.location().lat(), nr.location().lon()});
    }
    return it->second;
  }
};
