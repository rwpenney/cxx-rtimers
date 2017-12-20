/*
 *  Runtime timer classes
 *  RW Penney, December 2017
 */

#ifndef _CORE_HPP
#define _CORE_HPP

#include <cmath>
#include <ctime>
#include <iostream>


namespace rwputils {

template <typename MGR, typename LOG>
class Timer : protected MGR
{
  public:
    typedef typename MGR::clock_type clock_type;

    Timer(const std::string& name)
      : ident(name) {}
    ~Timer() {
      LOG::report(ident, stats);
    }

    void start() {
      MGR::recordStart(MGR::ClockProvider::now());
    }

    void stop() {
      const clock_type stopTime = MGR::ClockProvider::now();
      MGR::updateStats(stopTime, stats);
    }

  protected:
    const std::string ident;
    typename MGR::StatsAccumulator stats;
};


struct C89clock {
  typedef time_t clock_type;

  static time_t now() {
    return std::time(NULL);
  }

  static double interval(const time_t start, const time_t end) {
    return (end - start);
  }
};


struct NullManager
{
  typedef int clock_type;
  typedef int StatsAccumulator;

  struct ClockProvider {
    static clock_type now() { return 0; }
  };

  void recordStart(const clock_type& now) {}
  void updateStats(const clock_type& now, StatsAccumulator& stats) {}
};


template <typename CLK, typename STATS>
struct SerialManager
{
  typedef CLK ClockProvider;
  typedef STATS StatsAccumulator;
  typedef typename CLK::clock_type clock_type;

  void recordStart(const clock_type& now) {
    startTime = now;
  }

  void updateStats(const clock_type& now, STATS& stats) {
    const double duration = CLK::interval(startTime, now);
    stats.addSample(duration);
  }

  clock_type startTime;
};


struct BoundStats
{
  BoundStats()
    : count(0), tmin(1e18), tmax(-1e18) {}

  void addSample(double dt) {
    ++count;
    if (dt < tmin) tmin = dt;
    if (dt > tmax) tmax = dt;
  }

  unsigned long count;
  double tmin;
  double tmax;
};

std::ostream& operator<<(std::ostream& os, const BoundStats& stats) {
  os << stats.tmin << " <= t <= " << stats.tmax
     << " (n=" << stats.count << ")";
  return os;
}


struct MeanBoundStats : public BoundStats
{
  MeanBoundStats()
    : mean(0.0) {}

  void addSample(double dt) {
    BoundStats::addSample(dt);

    const double delta = dt - mean;
    mean += delta / count;
  }

  double mean;
};

std::ostream& operator<<(std::ostream& os, const MeanBoundStats& stats) {
  os << "<t> = " << stats.mean << " "
     << static_cast<BoundStats>(stats);
  return os;
}


struct VarBoundStats : public BoundStats
{
  VarBoundStats()
    : mean(0.0), nVariance(0.0) {}

  void addSample(double dt) {
    BoundStats::addSample(dt);

    const double delta = dt - mean;
    mean += delta / count;
    nVariance += ((count - 1) * delta) * delta / count;
  }

  double getStddev() const {
    return (count > 0 ? std::sqrt(nVariance / count) : 1e18);
  }

  double mean;
  double nVariance;
};

std::ostream& operator<<(std::ostream& os, const VarBoundStats& stats) {
  os << "<t> = " << stats.mean << " "
     << "std = " << stats.getStddev() << " "
     << static_cast<BoundStats>(stats);
  return os;
}


struct NullLogger
{
  template <typename STATS>
  static void report(const std::string& ident, const STATS& stats) {}
};


struct StdoutLogger
{
  template <typename STATS>
  static void report(const std::string& ident, const STATS& stats) {
    std::cout << "Timer(" << ident << "): " << stats << std::endl;
  }
};

}   // namespace rwputils

#endif  /* !_CORE_HPP */
