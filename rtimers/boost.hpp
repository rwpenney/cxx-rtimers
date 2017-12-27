/*
 *  Timer classes using boost::posix_time
 */

//  Copyright (C) 2017, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _RTIMERS_BOOST_HPP
#define _RTIMERS_BOOST_HPP

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "core.hpp"


namespace rtimers {
  namespace boostpt {


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


typedef Timer<SerialManager<HiResClock, VarBoundStats>,
              StderrLogger> DefaultTimer;
//typedef Timer<ThreadManager<HiResClock, VarBoundStats>,
//              StderrLogger> ThreadedTimer;


  }   // namespace boostpt
}   // namespace rtimers

#endif  /* !_RTIMERS_BOOST_HPP */
