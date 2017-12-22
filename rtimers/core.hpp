/*
 *  Runtime timer classes
 */

//  Copyright (C) 2017, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _RTIMERS_CORE_HPP
#define _RTIMERS_CORE_HPP

#include <cmath>
#include <ctime>
#include <iostream>


namespace rtimers {


/** Mechanism for automatically starting and stopping a timer
 *
 *  \see Timer
 */
template <typename TMR>
class ScopedStartStop
{
  public:
    ScopedStartStop(TMR& tmr)
      : timer(tmr) {
      timer.start();
    }
    ~ScopedStartStop() {
      timer.stop();
    }

  protected:
    TMR& timer;
};


/** Gather statistics from run-time stopwatch
 *
 *  This contains the core mechanisms for starting a stopping
 *  a timer. It is templated so that the management of
 *  thread-locking, statistics gathering and reporting
 *  can be customized.
 *
 *  It is assumed that all time-intervals are stored in units of seconds.
 *
 *  \see NullManager, SerialManager, MeanBoundStats, StderrLogger
 */
template <typename MGR, typename LOG>
class Timer : protected MGR
{
  public:
    typedef typename MGR::Instant Instant;
    typedef typename MGR::StatsAccumulator Stats;
    typedef Timer<MGR, LOG> self_t;
    typedef ScopedStartStop<self_t> Scoper;

    Timer(const std::string& name)
      : ident(name) {}
    ~Timer() {
      LOG::report(ident, stats);
    }

    //! Start the clock running
    void start() {
      MGR::recordStart(MGR::ClockProvider::now());
    }

    //! Stop the clock and accumulate time interval statistics
    void stop() {
      const Instant stopTime = MGR::ClockProvider::now();
      MGR::updateStats(stopTime, stats);
    }

    //! Create object which will start & stop the clock when in scope
    Scoper scopedStart() {
      return Scoper(*this);
    }

    //! Get current time-interval statistics (not thread safe)
    const Stats& getStats() const {
      return stats;
    }

  protected:
    //! An identifying label for this timer instance
    const std::string ident;

    //! A container for accumulating statistics on time intervals
    typename MGR::StatsAccumulator stats;
};


/** Low-precision wallclock time, using std::time()
 *
 *  This offers a resolution of one second.
 */
struct C89clock {
  typedef time_t Instant;

  static time_t now() {
    return std::time(NULL);
  }

  static double interval(const time_t start, const time_t end) {
    return (end - start);
  }
};


/** An empty timer-statistics controller
 *
 *  This gathers no statistics on interval times,
 *  and pays no attention to the system clock.
 *  It is intended to allow removal of timing functions,
 *  without changes to client code other than a simple typedef.
 */
struct NullManager
{
  typedef int Instant;
  typedef int StatsAccumulator;

  struct ClockProvider {
    static Instant now() { return 0; }
  };

  void recordStart(const Instant& now) {}
  void updateStats(const Instant& now, StatsAccumulator& stats) {}
};


/** Timer-statistics controller suitable for non-threaded code
 *
 *  \see C89clock, BoundStats
 */
template <typename CLK, typename STATS>
struct SerialManager
{
  typedef CLK ClockProvider;
  typedef STATS StatsAccumulator;
  typedef typename CLK::Instant Instant;

  //! Make a note of the time at which the stopwatch was started
  void recordStart(const Instant& now) {
    startTime = now;
  }

  //! Note the time the stopwatch was stopped, and accumulate statistics
  void updateStats(const Instant& now, STATS& stats) {
    const double duration = CLK::interval(startTime, now);
    stats.addSample(duration);
  }

  //! Most recent start time
  Instant startTime;
};


/** Accumulate simple min/max statistics of time intervals */
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


/** Accumulate min/max and average statistics of time intervals */
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
  os << "<t> = " << stats.mean << ", "
     << static_cast<BoundStats>(stats);
  return os;
}


/** Accumulate min/max, average and standard-deviation of time-intervals */
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
  os << "<t> = " << stats.mean << ", "
     << "std = " << stats.getStddev() << ", "
     << static_cast<BoundStats>(stats);
  return os;
}


/** Timer-statistics reporter which emits no output */
struct NullLogger
{
  template <typename STATS>
  static void report(const std::string& ident, const STATS& stats) {}
};


/** Timer-statistics reporter sending reports to std::cerr */
struct StderrLogger
{
  template <typename STATS>
  static void report(const std::string& ident, const STATS& stats) {
    std::cout << "Timer(" << ident << "): " << stats << std::endl;
  }
};


/** Timer-statistics reporter sending reports to single output stream
 *
 *  Client code will need to define a static field specifying the output stream, e.g.
 *  \code
 *  std::ostream& StreamLogger::stream = std::cout;
 *  \endcode
 */
class StreamLogger
{
  public:
    template <typename STATS>
    static void report(const std::string& ident, const STATS& stats) {
      stream << "Timer(" << ident << "): " << stats << std::endl;
    }

  protected:
    static std::ostream& stream;
};


typedef Timer<NullManager, NullLogger> NullTimer;
typedef Timer<SerialManager<C89clock, MeanBoundStats>,
              StderrLogger> BasicTimer;

}   // namespace rtimers

#endif  /* !_RTIMERS_CORE_HPP */
