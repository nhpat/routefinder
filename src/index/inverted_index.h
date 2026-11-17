#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/index/tokenizer.h"
#include "src/ingest/place.h"

struct SearchResult {
  uint32_t place_index;
  double score;
};

// token -> postings list, BM25-scored, AND semantics across query terms.
class InvertedIndex {
 public:
  void Build(const std::vector<Place>& places) {
    places_ = &places;
    doc_length_.assign(places.size(), 0);
    total_length_ = 0;
    postings_.clear();

    for (uint32_t i = 0; i < places.size(); ++i) {
      std::unordered_map<std::string, uint32_t> term_counts;
      for (const std::string& tok : Tokenize(places[i].name)) {
        ++term_counts[tok];
      }
      for (const auto& [key, value] : places[i].tags) {
        for (const std::string& tok : Tokenize(value)) {
          ++term_counts[tok];
        }
      }

      uint32_t length = 0;
      for (const auto& [term, count] : term_counts) {
        postings_[term].push_back({i, count});
        length += count;
      }
      doc_length_[i] = length;
      total_length_ += length;
    }

    for (auto& [term, list] : postings_) {
      std::sort(list.begin(), list.end(), [](const Posting& a, const Posting& b) {
        return a.place_index < b.place_index;
      });
    }
  }

  // Returns the top-k matches for places containing *every* query term,
  // ranked by summed BM25 score. Empty if any term is unknown.
  std::vector<SearchResult> Search(const std::string& query, uint32_t k) const {
    const std::vector<std::string> terms = Tokenize(query);
    if (terms.empty() || places_ == nullptr || places_->empty()) return {};

    std::vector<const std::vector<Posting>*> term_postings;
    term_postings.reserve(terms.size());
    for (const std::string& term : terms) {
      const auto it = postings_.find(term);
      if (it == postings_.end()) return {};  // AND semantics: every term must exist
      term_postings.push_back(&it->second);
    }

    std::sort(term_postings.begin(), term_postings.end(),
               [](const auto* a, const auto* b) { return a->size() < b->size(); });

    std::vector<uint32_t> candidates;
    candidates.reserve(term_postings.front()->size());
    for (const Posting& p : *term_postings.front()) candidates.push_back(p.place_index);

    for (size_t i = 1; i < term_postings.size() && !candidates.empty(); ++i) {
      candidates = IntersectSorted(candidates, *term_postings[i]);
    }
    if (candidates.empty()) return {};

    const double avg_length = static_cast<double>(total_length_) / places_->size();

    std::vector<SearchResult> results;
    results.reserve(candidates.size());
    for (uint32_t place_index : candidates) {
      double score = 0.0;
      for (const std::vector<Posting>* list : term_postings) {
        score += Bm25Term(*list, place_index, avg_length);
      }
      results.push_back({place_index, score});
    }

    const uint32_t top_k = std::min<uint32_t>(k, static_cast<uint32_t>(results.size()));
    std::partial_sort(results.begin(), results.begin() + top_k, results.end(),
                       [](const SearchResult& a, const SearchResult& b) {
                         return a.score > b.score;
                       });
    results.resize(top_k);
    return results;
  }

 private:
  struct Posting {
    uint32_t place_index;
    uint32_t term_frequency;
  };

  static constexpr double kK1 = 1.2;
  static constexpr double kB = 0.75;

  const std::vector<Place>* places_ = nullptr;
  std::unordered_map<std::string, std::vector<Posting>> postings_;
  std::vector<uint32_t> doc_length_;
  uint64_t total_length_ = 0;

  double Bm25Term(const std::vector<Posting>& list, uint32_t place_index,
                   double avg_length) const {
    const auto pit = std::lower_bound(
        list.begin(), list.end(), place_index,
        [](const Posting& p, uint32_t idx) { return p.place_index < idx; });
    if (pit == list.end() || pit->place_index != place_index) return 0.0;

    const uint32_t tf = pit->term_frequency;
    const double n = static_cast<double>(list.size());
    const double doc_count = static_cast<double>(places_->size());
    const double idf = std::log((doc_count - n + 0.5) / (n + 0.5) + 1.0);

    const double length_norm =
        (1.0 - kB) + kB * (doc_length_[place_index] / avg_length);
    const double tf_score = (tf * (kK1 + 1.0)) / (tf + kK1 * length_norm);

    return idf * tf_score;
  }

  static std::vector<uint32_t> IntersectSorted(const std::vector<uint32_t>& a,
                                                const std::vector<Posting>& b) {
    std::vector<uint32_t> result;
    size_t i = 0;
    size_t j = 0;
    while (i < a.size() && j < b.size()) {
      if (a[i] == b[j].place_index) {
        result.push_back(a[i]);
        ++i;
        ++j;
      } else if (a[i] < b[j].place_index) {
        ++i;
      } else {
        ++j;
      }
    }
    return result;
  }
};
