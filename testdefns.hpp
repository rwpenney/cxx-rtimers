/*
 *  Unit-test declarations for run-time timer classes
 */

//  Copyright (C) 2017, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <boost/test/unit_test.hpp>
#include "rtimers/core.hpp"

#if __cplusplus >= 201100
#  define RTIMERS_HAVE_CXX11 1
#else
#  define RTIMERS_HAVE_CXX11 0
#endif

#if defined(__unix) || defined(__linux)
#  define RTIMERS_HAVE_POSIX 1
#else
#  define RTIMERS_HAVE_POSIX 0
#endif


namespace rtimers {
  namespace testing {

constexpr double Pi = 3.14159265358979323846;

//! Consume some CPU time, starting and stopping timer a fixed number of times
template <typename TMR>
double occupyTimer(TMR& timer, unsigned iterations) {
  double tot = 0.0;

  for (unsigned n=0; n<iterations; ++n) {
    typename TMR::Scoper scoper = timer.scopedStart();

    tot += std::cos((n * 252 + 23) % 59);
  }

  return tot;
}


struct TestCxx11 : boost::unit_test::test_suite
{
  TestCxx11();

  static void serial();
  static void threaded();
};


struct TestPosix : boost::unit_test::test_suite
{
  TestPosix();

  static void serial();
  static void threaded();
};


  }   // namespace testing
}   // namespace rtimers
