#include <iostream>
#include <string>

#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>

#include "src/ingest/binary_io.h"
#include "src/ingest/places_handler.h"
#include "src/ingest/road_graph_handler.h"

namespace {

void RunPlacesPass(const std::string& input_path) {
  // osm_entity_bits::node tells the reader to decode only nodes, skipping
  // way/relation blocks entirely during this pass.
  osmium::io::Reader reader{input_path, osmium::osm_entity_bits::node};
  PlacesHandler handler;
  osmium::apply(reader, handler);
  reader.close();

  routefinder::io::WritePlaces("data/places.bin", handler.places);
  std::cout << "places: " << handler.places.size() << "\n";
}

void RunRoadGraphPass(const std::string& input_path) {
  using Index =
      osmium::index::map::FlexMem<osmium::unsigned_object_id_type,
                                   osmium::Location>;
  using LocationHandler = osmium::handler::NodeLocationsForWays<Index>;

  Index index;
  LocationHandler location_handler{index};
  // The NYC extract is clipped to a bounding box, so some ways near the
  // boundary reference nodes outside the file; don't throw on those.
  location_handler.ignore_errors();

  osmium::io::Reader reader{
      input_path, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
  RoadGraphHandler road_handler;
  // location_handler runs first so each way's node refs already carry
  // resolved locations by the time road_handler.way() sees them.
  osmium::apply(reader, location_handler, road_handler);
  reader.close();

  routefinder::io::WriteGraph("data/graph.bin", road_handler.nodes,
                               road_handler.edges);
  std::cout << "graph nodes: " << road_handler.nodes.size() << "\n";
  std::cout << "graph edges: " << road_handler.edges.size() << "\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: ingest <path-to.osm.pbf>\n";
    return 1;
  }
  const std::string input_path = argv[1];

  RunPlacesPass(input_path);
  RunRoadGraphPass(input_path);

  return 0;
}
