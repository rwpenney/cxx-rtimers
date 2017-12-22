/*
 *  Timer classes using C++11 std::chrono
 */

#ifndef _RTIMERS_CXX11_HPP
#define _RTIMERS_CXX11_HPP

#include <chrono>

#include "core.hpp"


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


using DefaultTimer = Timer<SerialManager<HiResClock, VarBoundStats>,
                           StderrLogger>;

  }   // namespace cxx11
}   // namespace rtimers


#endif  /* !_RTIMERS_CXX11_HPP */
