#pragma once

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;

class Leaderboard {
 private:
  Leaderboard() {
    minScore = numeric_limits<double>::min();
  }
  Leaderboard(const char* level) {
    minScore = numeric_limits<double>::min();
    name = string(level, strlen(level));
  }

  friend class boost::serialization::access;
  template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
    ar & name;
    ar & minScore;
    ar & highscores;
  }

  string name;
  double minScore;
  multimap<double, string> highscores;

 public:
  static Leaderboard& findLeaderboard(const char* level) {
    stringstream ss;
    ss << level << ".obhs";
    std::ifstream ifs(ss.str(), std::ios::binary);
    if (ifs) {
      boost::archive::binary_iarchive ia(ifs);
      Leaderboard *l = new Leaderboard();
      ia >> *l;
      return *l;
    } else {
      Leaderboard *l = new Leaderboard(level);
      return *l;
    }
  }

  void saveToFile() {
    stringstream ss;
    ss << name << ".obhs";
    std::ofstream ofs(ss.str(), std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << *this;
  }

  bool isHighscore(double score) {
    return highscores.size() < 10 || score > minScore;
  }

  void addHighscore(const char* player, double score) {
    // Delete minimum element
    if (highscores.size() == 10) {
      multimap<double, string>::iterator minElem = highscores.find(minScore);
      auto item = minElem->first;
      minElem++;
      highscores.erase(item);
    }

    highscores.emplace(score, string(player, strlen(player)));

    // Find new minimum score
    minScore = score;

    multimap<double, string>::iterator iter;
    for (iter = highscores.begin(); iter != highscores.end();) {
      if (iter->first < minScore)
        minScore = iter->first;
    }
  }

  multimap<double, string> getHighscores() {
    return highscores;
  }
};
