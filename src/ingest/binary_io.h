#pragma once

#include <cstdint>
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
