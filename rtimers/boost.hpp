/*
 *  Timer classes using boost::posix_time
 */

//  (C)Copyright 2017-2021, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _RTIMERS_BOOST_HPP
#define _RTIMERS_BOOST_HPP

#include <boost/core/noncopyable.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

#include "core.hpp"


namespace rtimers {
  namespace boostpt {


/** System clock offering at least microsecond resolution */
struct HiResClock
{
  typedef ::boost::posix_time::microsec_clock Provider;
  typedef ::boost::posix_time::ptime Instant;

  static Instant now() {
    return Provider::universal_time();
  }

  static double interval(const Instant& start, const Instant& end) {
    const ::boost::posix_time::time_duration delta = (end - start);
    return delta.total_seconds() + (delta.fractional_seconds()
                                      / (double)delta.ticks_per_second());
  }
};


/** Timer-statistics controller suitable for threaded code
 *
 *  \see cxx11::ThreadManager, SerialManager, HiResClock
 */
template <typename CLK, typename STATS>
class ThreadManager : boost::noncopyable
{
  public:
    typedef CLK ClockProvider;
    typedef STATS StatsAccumulator;
    typedef typename CLK::Instant Instant;
    typedef ThreadManager<CLK, STATS> self_t;
    typedef std::map<self_t*, Instant> TimeMap;

    //! Make a note of the time at which the stopwatch was started
    void recordStart(const Instant& dummy) {
      TimeMap* timemap = startTimes.get();
      if (!timemap) {
        timemap = new TimeMap;
        startTimes.reset(timemap);
      }
      (*timemap)[this] = dummy;

      const typename TimeMap::const_iterator itr = timemap->find(this);
      timemap->insert(itr, { this, CLK::now() });
    }

    //! Note the time the stopwatch was stopped, and accumulate statistics
    void updateStats(const Instant& now, STATS& stats) {
      TimeMap* timemap = startTimes.get();
      const double duration = CLK::interval(timemap->at(this), now);

      {
        boost::mutex::scoped_lock lock(stats_mtx);
        stats.addSample(duration);
      }
    }

  protected:
    static boost::thread_specific_ptr<TimeMap> startTimes;

    boost::mutex stats_mtx;
};

template <typename CLK, typename STATS>
boost::thread_specific_ptr<typename ThreadManager<CLK, STATS>::TimeMap> ThreadManager<CLK, STATS>::startTimes;


typedef Timer<SerialManager<HiResClock, VarBoundStats>,
              StderrLogger> DefaultTimer;
typedef Timer<ThreadManager<HiResClock, VarBoundStats>,
              StderrLogger> ThreadedTimer;


  }   // namespace boostpt
}   // namespace rtimers

#define RTIMERS_BOOST_STATIC_SCOPED(name) \
  static rtimers::boostpt::DefaultTimer _rtimers_tmr_bpt(name); \
  rtimers::boostpt::DefaultTimer::Scoper _rtimers_scp_bpt = _rtimers_tmr_bpt.scopedStart();
#ifndef RTIMERS_STATIC_SCOPED
#  define RTIMERS_STATIC_SCOPED(name) RTIMERS_BOOST_STATIC_SCOPED(name)
#endif

#endif  /* !_RTIMERS_BOOST_HPP */
