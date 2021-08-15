/*
 *  Runtime timer classes
 */

//  (C)Copyright 2017-2021, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _RTIMERS_CORE_HPP
#define _RTIMERS_CORE_HPP

#include <cmath>
#include <ctime>
#include <iostream>


namespace rtimers {

struct BoundStats;
struct MeanBoundStats;
struct TimeUnit;
struct VarBoundStats;


//! Estimate the time delay between adjacent queries of system clock
template <typename CLK, typename STATS=MeanBoundStats>
STATS clockZeroError(unsigned iterations=1000) {
  STATS zeros;

  for (unsigned i=0; i<iterations; ++i) {
    const typename CLK::Instant t0 = CLK::now();
    const typename CLK::Instant t1 = CLK::now();
    zeros.addSample(CLK::interval(t0, t1));
  }

  return zeros;
}


/** Mechanism for automatically starting and stopping a timer
 *
 *  \see Timer::scopedStart()
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
 *  This contains the core mechanisms for starting and stopping
 *  a timer. It is templated so that the management of
 *  thread-locking, statistics gathering and reporting
 *  can be customized.
 *
 *  It is assumed that all time-intervals are stored in units of seconds.
 *
 *  \see NullManager, SerialManager, StderrLogger
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

    /*! Estimate time delay between adjacent queries of system clock
     *
     *  \see MeanBoundStats
     */
    template <typename STATS>
    static STATS zeroError(unsigned iterations=1000) {
      return clockZeroError<typename MGR::ClockProvider, STATS>(iterations);
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
 *
 *  \see cxx11::HiResClock.
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
 *
 *  \see NullTimer.
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


struct TimeUnit {
  TimeUnit(const std::string& u, double m)
    : unit(u), mult(1.0 / m) {}

  const std::string unit;   //!< The time unit (e.g. "us", "s")
  const double mult;        //!< Multiplier to convert from seconds
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

  static TimeUnit guessUnit(double tscale) {
    if (tscale == 0.0) {
      return TimeUnit("s", 1.0);
    } else if (tscale < 250e-9) {
      return TimeUnit("ns", 1e-9);
    } else if (tscale < 250e-6) {
      return TimeUnit("us", 1e-6);
    } else if (tscale < 250e-3) {
      return TimeUnit("ms", 1e-3);
    } else if (tscale < 400) {
      return TimeUnit("s", 1.0);
    } else if (tscale < 7500) {
      return TimeUnit("m", 60.0);
    } else {
      return TimeUnit("h", 3600.0);
    }
  }
};

inline std::ostream& operator<<(std::ostream& os,
                                const BoundStats& stats) {
  const TimeUnit tu = stats.guessUnit(0.5 * (stats.tmin + stats.tmax));

  os << (stats.tmin * tu.mult) << tu.unit
     << " <= t <= "
     << (stats.tmax * tu.mult) << tu.unit
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

inline std::ostream& operator<<(std::ostream& os,
                                const MeanBoundStats& stats) {
  const TimeUnit tu = stats.guessUnit(stats.mean);

  os << "<t> = " << (stats.mean * tu.mult) << tu.unit << ", "
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

inline std::ostream& operator<<(std::ostream& os,
                                const VarBoundStats& stats) {
  const TimeUnit tu = stats.guessUnit(stats.mean);

  os << "<t> = " << (stats.mean * tu.mult) << tu.unit << ", "
     << "std = " << (stats.getStddev() * tu.mult) << tu.unit << ", "
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
