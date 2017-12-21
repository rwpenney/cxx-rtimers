/*
 *  Timer classes using C++11 std::chrono
 */

#ifndef _RTIMERS_CHRONO_HPP
#define _RTIMERS_CHRONO_HPP

#include <chrono>

#include "core.hpp"


namespace rtimers {
  namespace chrono {

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

  }   // namespace chrono
}   // namespace rtimers


#endif  /* !_RTIMERS_CHRONO_HPP */
