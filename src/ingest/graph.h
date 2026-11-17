#pragma once

#include <cstdint>

struct GraphNode {
  double lat = 0;
  double lng = 0;
};

struct GraphEdge {
  uint32_t from = 0;
  uint32_t to = 0;
  double weight_s = 0;
};
