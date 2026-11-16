#pragma once

#include <algorithm>
#include <array>

#include <osmium/handler.hpp>
#include <osmium/osm/node.hpp>

#include "src/ingest/place.h"

// Keeps OSM nodes that look like a searchable point of interest: they must
// have a name, and at least one of the tag keys below. Everything else
// (way-shape vertices, bare intersections, traffic furniture, ...) is
// dropped during the streaming pass instead of being materialized.
class PlacesHandler : public osmium::handler::Handler {
 public:
  std::vector<Place> places;

  void node(const osmium::Node& n) {
    const osmium::TagList& tags = n.tags();

    const char* name = tags["name"];
    if (name == nullptr) return;

    static constexpr std::array<const char*, 4> kPlaceTagKeys = {
        "amenity", "shop", "tourism", "leisure"};
    const bool is_place =
        std::any_of(kPlaceTagKeys.begin(), kPlaceTagKeys.end(),
                     [&](const char* key) { return tags[key] != nullptr; });
    if (!is_place) return;

    Place place;
    place.id = n.id();
    place.lat = n.location().lat();
    place.lng = n.location().lon();
    place.name = name;
    for (const osmium::Tag& tag : tags) {
      place.tags.emplace_back(tag.key(), tag.value());
    }
    places.push_back(std::move(place));
  }
};
