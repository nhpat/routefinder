#pragma once

#include <cctype>
#include <string>
#include <vector>

// Lowercases and splits on any non-alphanumeric character, so "Blue Bottle
// Coffee!" -> ["blue", "bottle", "coffee"].
inline std::vector<std::string> Tokenize(const std::string& text) {
  std::vector<std::string> tokens;
  std::string current;

  for (char c : text) {
    if (std::isalnum(static_cast<unsigned char>(c))) {
      current.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    } else if (!current.empty()) {
      tokens.push_back(std::move(current));
      current.clear();
    }
  }
  if (!current.empty()) tokens.push_back(std::move(current));

  return tokens;
}
