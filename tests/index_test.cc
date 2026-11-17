#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "src/index/inverted_index.h"
#include "src/index/tokenizer.h"

TEST(Tokenizer, LowercasesAndSplitsOnPunctuation) {
  EXPECT_EQ(Tokenize("Blue Bottle Coffee!"),
            (std::vector<std::string>{"blue", "bottle", "coffee"}));
}

TEST(Tokenizer, EmptyStringProducesNoTokens) {
  EXPECT_TRUE(Tokenize("").empty());
}

namespace {

std::vector<Place> MakeTestPlaces() {
  std::vector<Place> places;

  Place blue_bottle;
  blue_bottle.id = 1;
  blue_bottle.name = "Blue Bottle Coffee";
  blue_bottle.tags = {{"amenity", "cafe"}};
  places.push_back(blue_bottle);

  Place joe_coffee;
  joe_coffee.id = 2;
  joe_coffee.name = "Joe Coffee";
  joe_coffee.tags = {{"amenity", "cafe"}};
  places.push_back(joe_coffee);

  Place pizza_place;
  pizza_place.id = 3;
  pizza_place.name = "Joe's Pizza";
  pizza_place.tags = {{"amenity", "restaurant"}, {"cuisine", "pizza"}};
  places.push_back(pizza_place);

  return places;
}

}  // namespace

TEST(InvertedIndex, MatchesOnTagValueNotJustName) {
  const std::vector<Place> places = MakeTestPlaces();
  InvertedIndex index;
  index.Build(places);

  const std::vector<SearchResult> results = index.Search("cafe", 10);
  ASSERT_EQ(results.size(), 2u);  // both coffee shops are tagged amenity=cafe
  for (const SearchResult& r : results) {
    EXPECT_NE(places[r.place_index].name, "Joe's Pizza");
  }
}

TEST(InvertedIndex, MultiWordQueryRequiresAllTerms) {
  const std::vector<Place> places = MakeTestPlaces();
  InvertedIndex index;
  index.Build(places);

  const std::vector<SearchResult> results = index.Search("joe coffee", 10);
  ASSERT_EQ(results.size(), 1u);
  EXPECT_EQ(places[results[0].place_index].name, "Joe Coffee");
}

TEST(InvertedIndex, UnknownTermReturnsNoResults) {
  const std::vector<Place> places = MakeTestPlaces();
  InvertedIndex index;
  index.Build(places);

  EXPECT_TRUE(index.Search("sushi", 10).empty());
}

TEST(InvertedIndex, ShorterDocumentScoresHigherForEqualTermFrequency) {
  const std::vector<Place> places = MakeTestPlaces();
  InvertedIndex index;
  index.Build(places);

  const std::vector<SearchResult> results = index.Search("coffee", 10);
  ASSERT_EQ(results.size(), 2u);
  // "Joe Coffee" (shorter doc) should outrank "Blue Bottle Coffee" (longer
  // doc) despite both matching "coffee" exactly once each, due to BM25's
  // length normalization.
  EXPECT_EQ(places[results[0].place_index].name, "Joe Coffee");
}
