#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/ingest/graph.h"
#include "src/ingest/place.h"

namespace routefinder::io {

namespace detail {

inline void WriteU32(std::ofstream& out, uint32_t v) {
  out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

inline void WriteU64(std::ofstream& out, uint64_t v) {
  out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

inline void WriteDouble(std::ofstream& out, double v) {
  out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

inline void WriteString(std::ofstream& out, const std::string& s) {
  WriteU32(out, static_cast<uint32_t>(s.size()));
  out.write(s.data(), static_cast<std::streamsize>(s.size()));
}

inline uint32_t ReadU32(std::ifstream& in) {
  uint32_t v = 0;
  in.read(reinterpret_cast<char*>(&v), sizeof(v));
  return v;
}

inline uint64_t ReadU64(std::ifstream& in) {
  uint64_t v = 0;
  in.read(reinterpret_cast<char*>(&v), sizeof(v));
  return v;
}

inline double ReadDouble(std::ifstream& in) {
  double v = 0;
  in.read(reinterpret_cast<char*>(&v), sizeof(v));
  return v;
}

inline std::string ReadString(std::ifstream& in) {
  const uint32_t len = ReadU32(in);
  std::string s(len, '\0');
  in.read(s.data(), static_cast<std::streamsize>(len));
  return s;
}

}  // namespace detail

// Layout: "RFPL" magic, u32 version, u64 count, then per place:
// u64 id, f64 lat, f64 lng, string name, u32 tag_count, (string key, string
// value) * tag_count.
inline void WritePlaces(const std::string& path,
                         const std::vector<Place>& places) {
  std::ofstream out(path, std::ios::binary);
  if (!out) throw std::runtime_error("could not open " + path + " for writing");

  out.write("RFPL", 4);
  detail::WriteU32(out, /*version=*/1);
  detail::WriteU64(out, places.size());

  for (const Place& p : places) {
    detail::WriteU64(out, p.id);
    detail::WriteDouble(out, p.lat);
    detail::WriteDouble(out, p.lng);
    detail::WriteString(out, p.name);
    detail::WriteU32(out, static_cast<uint32_t>(p.tags.size()));
    for (const auto& [key, value] : p.tags) {
      detail::WriteString(out, key);
      detail::WriteString(out, value);
    }
  }
}

inline std::vector<Place> ReadPlaces(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("could not open " + path + " for reading");

  char magic[4];
  in.read(magic, 4);
  if (std::memcmp(magic, "RFPL", 4) != 0) {
    throw std::runtime_error(path + " is not a valid places.bin file");
  }
  detail::ReadU32(in);  // version, unused for now
  const uint64_t count = detail::ReadU64(in);

  std::vector<Place> places;
  places.reserve(count);
  for (uint64_t i = 0; i < count; ++i) {
    Place p;
    p.id = detail::ReadU64(in);
    p.lat = detail::ReadDouble(in);
    p.lng = detail::ReadDouble(in);
    p.name = detail::ReadString(in);
    const uint32_t tag_count = detail::ReadU32(in);
    p.tags.reserve(tag_count);
    for (uint32_t t = 0; t < tag_count; ++t) {
      std::string key = detail::ReadString(in);
      std::string value = detail::ReadString(in);
      p.tags.emplace_back(std::move(key), std::move(value));
    }
    places.push_back(std::move(p));
  }
  return places;
}

// Layout: "RFGR" magic, u32 version, u64 node_count, u64 edge_count, then
// node_count * (f64 lat, f64 lng), then edge_count * (u32 from, u32 to,
// f64 weight_s).
inline void WriteGraph(const std::string& path,
                        const std::vector<GraphNode>& nodes,
                        const std::vector<GraphEdge>& edges) {
  std::ofstream out(path, std::ios::binary);
  if (!out) throw std::runtime_error("could not open " + path + " for writing");

  out.write("RFGR", 4);
  detail::WriteU32(out, /*version=*/1);
  detail::WriteU64(out, nodes.size());
  detail::WriteU64(out, edges.size());

  for (const GraphNode& n : nodes) {
    detail::WriteDouble(out, n.lat);
    detail::WriteDouble(out, n.lng);
  }
  for (const GraphEdge& e : edges) {
    detail::WriteU32(out, e.from);
    detail::WriteU32(out, e.to);
    detail::WriteDouble(out, e.weight_s);
  }
}

}  // namespace routefinder::io
