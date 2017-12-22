/*
 *  Unit-test declarations for run-time timer classes
 */

//  Copyright (C) 2017, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <boost/test/unit_test.hpp>

#if __cplusplus >= 201100
#  define RTIMERS_HAVE_CXX11 1
#else
#  define RTIMERS_HAVE_CXX11 0
#endif


namespace rtimers {
  namespace testing {

constexpr double Pi = 3.14159265358979323846;


struct TestCxx11 : boost::unit_test::test_suite
{
  TestCxx11();

  static void threaded();
};


  }   // namespace testing
}   // namespace rtimers
