#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct Place {
  uint64_t id = 0;
  double lat = 0;
  double lng = 0;
  std::string name;
  std::vector<std::pair<std::string, std::string>> tags;
};
