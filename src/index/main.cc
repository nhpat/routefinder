#include <cstdlib>
#include <iostream>
#include <string>

#include "src/index/inverted_index.h"
#include "src/ingest/binary_io.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "usage: search_cli <places.bin> <query> [k]\n";
    return 1;
  }
  const std::string places_path = argv[1];
  const std::string query = argv[2];
  const uint32_t k = argc > 3 ? static_cast<uint32_t>(std::stoul(argv[3])) : 5;

  const std::vector<Place> places = routefinder::io::ReadPlaces(places_path);

  InvertedIndex index;
  index.Build(places);

  const std::vector<SearchResult> results = index.Search(query, k);
  if (results.empty()) {
    std::cout << "no results\n";
    return 0;
  }

  for (const SearchResult& r : results) {
    const Place& p = places[r.place_index];
    std::cout << p.name << "  score=" << r.score << "  lat=" << p.lat
               << " lng=" << p.lng << "\n";
  }
  return 0;
}
