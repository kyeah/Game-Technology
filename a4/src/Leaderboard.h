#pragma once

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <ctime>

using namespace std;

class LeaderboardEntry {
 public:
  LeaderboardEntry() {}
 LeaderboardEntry(const char* player, double _score, double _timeTaken, time_t _timeEntered)
   : score(_score), timeTaken(_timeTaken), timeEntered(_timeEntered) {
    name = string(player, strlen(player));
  }

  friend class boost::serialization::access;
  template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
    ar & name;
    ar & score;
    ar & timeTaken;
    ar & timeEntered;
  }

  string getTimeTaken() {
    std::stringstream timess;
    int seconds = std::round(timeTaken/1000);
    int millis = std::min(99.0f, (float)std::round(fmod(timeTaken,1000)/10));
    timess << seconds << ":";
    if (millis < 10) timess << "0";
    timess << millis;

    return timess.str();
  }

  string getTimeEntered() {
    struct tm *time = localtime(&timeEntered);

    char buf[25];
    bzero(buf, 25);
    strftime(buf, 25, "%B %d, %Y", time);
    return string(buf, 25);
  }

  string name;
  double score;
  double timeTaken;
  time_t timeEntered;
};

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
  multimap<double, LeaderboardEntry, greater<double> > highscores;

 public:
  static Leaderboard& findLeaderboard(const char* level) {
    stringstream ss;
    ss << "data/" << level << ".obhs";
    ifstream ifs(ss.str(), std::ios::binary);
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
    ss << "data/" << name << ".obhs";
    std::ofstream ofs(ss.str(), std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << *this;
  }

  bool isHighscore(double score) {
    return highscores.size() < 10 || score > minScore;
  }

  void addHighscore(const char* player, double score, double timeTaken) {
    if (!isHighscore(score)) return;

    // Delete minimum element
    if (highscores.size() == 10) {
      multimap<double, LeaderboardEntry, greater<double> >::iterator minElem = highscores.find(minScore);
      auto item = minElem->first;
      minElem++;
      highscores.erase(item);
    }

    highscores.insert(pair<double, LeaderboardEntry>(score, LeaderboardEntry(player,
                                                                             score,
                                                                             timeTaken,
                                                                             time(0))));

    // Find new minimum score
    minScore = score;

    multimap<double, LeaderboardEntry>::iterator iter;
    for (iter = highscores.begin(); iter != highscores.end(); iter++) {
      if (iter->first < minScore)
        minScore = iter->first;
    }
  }

  multimap<double, LeaderboardEntry, greater<double> > getHighscores() {
    return highscores;
  }
};
