#include <iostream>
#include <string>

#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>

#include "src/ingest/binary_io.h"
#include "src/ingest/places_handler.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: ingest <path-to.osm.pbf>\n";
    return 1;
  }
  const std::string input_path = argv[1];

  // osm_entity_bits::node tells the reader to decode only nodes, skipping
  // way/relation blocks entirely during this pass.
  osmium::io::Reader reader{input_path, osmium::osm_entity_bits::node};
  PlacesHandler handler;
  osmium::apply(reader, handler);
  reader.close();

  routefinder::io::WritePlaces("data/places.bin", handler.places);

  std::cout << "places: " << handler.places.size() << "\n";
  return 0;
}
