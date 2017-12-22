/*
 *  Timer classes using POSIX clock_gettime() functions
 */

#ifndef _RTIMERS_POSIX_HPP
#define _RTIMERS_POSIX_HPP

#include <map>
#include <pthread.h>
#include <time.h>

#include "core.hpp"


namespace rtimers {
  namespace posix {


/** POSIX system clock based on clock_gettime() */
struct HiResClock {
  typedef timespec Instant;

  static Instant now() {
    Instant t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t;
  }

  static double interval(const Instant& start, const Instant& end) {
    return (end.tv_sec - start.tv_sec)
            + (end.tv_nsec - start.tv_nsec) * 1e-9;
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
    typedef CLK ClockProvider;
    typedef STATS StatsAccumulator;
    typedef typename CLK::Instant Instant;
    typedef ThreadManager<CLK, STATS> self_t;

    ThreadManager() {
      pthread_once(&once_ctrl, init_once);
      pthread_mutex_init(&stats_mtx, NULL);
    }
    ~ThreadManager() {
      pthread_mutex_destroy(&stats_mtx);
    }

    void recordStart(const Instant& dummy) {
      pthread_mutex_lock(&stats_mtx);

      TimeMap* startTimes = static_cast<TimeMap*>(
                                              pthread_getspecific(start_key));
      if (startTimes == NULL) {
        startTimes = new TimeMap;
        pthread_setspecific(start_key, startTimes);
      }
      (*startTimes)[this] = dummy;

      pthread_mutex_unlock(&stats_mtx);

      const typename TimeMap::const_iterator itr = startTimes->find(this);
      startTimes->insert(itr, typename TimeMap::value_type(this, CLK::now()));
    }

    void updateStats(const Instant& now, STATS& stats) {
      pthread_mutex_lock(&stats_mtx);

      TimeMap* startTimes = static_cast<TimeMap*>(
                                              pthread_getspecific(start_key));
      const double duration = CLK::interval((*startTimes)[this], now);

      stats.addSample(duration);
      pthread_mutex_unlock(&stats_mtx);
    }

  protected:
    typedef std::map<self_t*, Instant> TimeMap;

    static pthread_key_t start_key;
    static pthread_once_t once_ctrl;
    pthread_mutex_t stats_mtx;

    static void init_once() {
      pthread_key_create(&start_key, tmapDelete);
    }

    static void tmapDelete(void* tm) {
      delete static_cast<TimeMap*>(tm);
    }
};

template <typename CLK, typename STATS>
pthread_key_t ThreadManager<CLK, STATS>::start_key;

template <typename CLK, typename STATS>
pthread_once_t ThreadManager<CLK, STATS>::once_ctrl;


typedef Timer<SerialManager<HiResClock, VarBoundStats>,
                            StderrLogger> DefaultTimer;
typedef Timer<ThreadManager<HiResClock, VarBoundStats>,
                            StderrLogger> ThreadedTimer;


  }   // namespace posix
}   // namespace rtimers


#endif  /* !_RTIMERS_POSIX_HPP */
