/*
 *  Timer classes using C++11 std::chrono
 */

#ifndef _RTIMERS_CXX11_HPP
#define _RTIMERS_CXX11_HPP

#include <chrono>
#include <map>
#include <mutex>

#include "core.hpp"


#if __cplusplus < 201100
#  error "rtimers/cxx11 requires C++11 support"
#endif


namespace rtimers {
  namespace cxx11 {

/** System clock offering highest available time resolution */
struct HiResClock
{
  using Provider = std::chrono::high_resolution_clock;
  using Instant = Provider::time_point;

  static Instant now() {
    return Provider::now();
  }

  static double interval(const Instant& start, const Instant& end) {
    const std::chrono::duration<double> interval = end - start;
    return interval.count();
  }
};


/** Timer-statistics controller suitable for threaded code
 *
 *  Note that the overheads associated with mutex locks,
 *  and thread-local storage mean that the zero-offset
 *  of these timers are likely to be hundreds of nanoseconds.
 *
 *  \see SerialManager, HiResClock, BoundStats.
 */
template <typename CLK, typename STATS>
class ThreadManager
{
  public:
    using ClockProvider = CLK;
    using StatsAccumulator = STATS;
    using Instant = typename CLK::Instant;
    using self_t = ThreadManager<CLK, STATS>;

    //! Make a note of the time at which the stopwatch was started
    void recordStart(const Instant& dummy) {
      std::lock_guard<std::mutex> lock(time_mtx);

      // Insert dummy start-time, which may be slow because this
      // involves a map lookup based on an instance address:
      startTimes[this] = dummy;

      // Attempt to record a more accurate start-time:
      const auto itr = startTimes.find(this);
      startTimes.insert(itr, { this, CLK::now() });
    }

    //! Note the time the stopwatch was stopped, and accumulate statistics
    void updateStats(const Instant& now, STATS& stats) {
      const double duration = CLK::interval(startTimes[this], now);

      {
        std::lock_guard<std::mutex> lock(stats_mtx);
        stats.addSample(duration);
      }
    }

  protected:
    //! Most recent start times
    thread_local static std::map<self_t*, Instant> startTimes;

    std::mutex time_mtx, stats_mtx;
};

template <typename CLK, typename STATS>
thread_local std::map<ThreadManager<CLK, STATS>*, typename CLK::Instant>
  ThreadManager<CLK, STATS>::startTimes;


using DefaultTimer = Timer<SerialManager<HiResClock, VarBoundStats>,
                           StderrLogger>;
using ThreadedTimer = Timer<ThreadManager<HiResClock, VarBoundStats>,
                           StderrLogger>;

  }   // namespace cxx11
}   // namespace rtimers


#endif  /* !_RTIMERS_CXX11_HPP */
