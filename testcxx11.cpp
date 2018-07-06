/*
 *  C++11 unit-tests for run-time timers
 */

//  Copyright (C) 2017, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <boost/test/unit_test.hpp>
#include <cmath>

#include "testdefns.hpp"

#if RTIMERS_HAVE_CXX11
#  include <memory>
#  include <random>
#  include <thread>
#  include <vector>
#  include "rtimers/cxx11.hpp"
#endif

namespace BoostUT = boost::unit_test;


namespace rtimers {
  namespace testing {

#if RTIMERS_HAVE_CXX11

using QuietSerialTimer = Timer<SerialManager<cxx11::HiResClock,
                                             MeanBoundStats>,
                               NullLogger>;
using QuietThreadedTimer = Timer<cxx11::ThreadManager<cxx11::HiResClock,
                                                      MeanBoundStats>,
                                 NullLogger>;

static void keepBusy(QuietThreadedTimer* timer, unsigned iterations)
{
  occupyTimer(*timer, iterations);
}

#endif  // RTIMERS_HAVE_CXX11


TestCxx11::TestCxx11()
  : BoostUT::test_suite("C++11 timer variants")
{
  add(BOOST_TEST_CASE(serial));
  add(BOOST_TEST_CASE(threaded));
}


void TestCxx11::serial()
{
#if RTIMERS_HAVE_CXX11
  QuietSerialTimer tmr("C++ chrono");
  double tot = 0.0;
  const unsigned iterations = 100;

  for (unsigned i=0; i<iterations; ++i) {
    tmr.start();
    tot += std::log(std::pow(1.5 + std::cos(i * 1.3), 2.73));
    // Assume that this takes at least 50ns
    tmr.stop();
  }
  BOOST_CHECK(tot != 0.0);

  const auto stats = tmr.getStats();
  BOOST_CHECK_EQUAL(stats.count, iterations);
  BOOST_CHECK_GT(stats.mean, 50e-9);
  BOOST_CHECK_LT(stats.tmax, 1.0);

  auto zeros = tmr.zeroError<MeanBoundStats>();

  BOOST_CHECK_GT(zeros.mean, 1e-9);
  BOOST_CHECK_LT(zeros.tmax, 0.1);
  BOOST_CHECK_GT(stats.mean, zeros.tmin + 50e-9);
#else // !RTIMERS_HAVE_CXX11
  BOOST_ERROR("No C++11 chrono support");
#endif  // RTIMERS_HAVE_CXX11
}


void TestCxx11::threaded()
{
#if RTIMERS_HAVE_CXX11
  std::vector<std::unique_ptr<std::thread>> threads;
  std::vector<unsigned> expected;
  std::vector<std::unique_ptr<QuietThreadedTimer>> timers;
  std::ranlux24_base randeng;
  std::uniform_int_distribution<unsigned> uniformdist(200, 50000);
  const unsigned ntimers = 7, nthreads = 200;

  for (unsigned i=0; i<ntimers; ++i) {
    std::stringstream ident;
    ident << "C++11 threads [" << i << "]";

    timers.push_back(std::unique_ptr<QuietThreadedTimer>(
                            new QuietThreadedTimer(ident.str())));
    expected.push_back(0);
  }

  for (unsigned i=0; i<nthreads; ++i) {
    unsigned slot = i % ntimers;
    QuietThreadedTimer *tmr = timers[slot].get();
    unsigned iterations = uniformdist(randeng);

    threads.push_back(std::unique_ptr<std::thread>(
                            new std::thread(keepBusy, tmr, iterations)));
    expected[slot] += iterations;
  }

  while (!threads.empty()) {
    threads.back()->join();
    threads.pop_back();
  }

  for (unsigned i=0; i<ntimers; ++i) {
    BOOST_CHECK_EQUAL(expected[i], timers[i]->getStats().count);
  }
#else // !RTIMERS_HAVE_CXX11
  BOOST_ERROR("No C++11 multi-thread support");
#endif  // RTIMERS_HAVE_CXX11
}


  }   // namespace testing
}   // namespace rtimers
