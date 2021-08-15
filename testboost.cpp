/*
 *  Unit-tests for timers using boost::posix_time
 */

//  (C)Copyright 2017-2021, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <cmath>
#include <cstdlib>

#include "testdefns.hpp"
#include "rtimers/boost.hpp"

namespace BoostUT = boost::unit_test;


namespace rtimers {
  namespace testing {


typedef Timer<boostpt::ThreadManager<boostpt::HiResClock, MeanBoundStats>,
                                     NullLogger> QuietThreadedTimer;

static void keepBusy(QuietThreadedTimer* timer, unsigned iterations)
{
  occupyTimer(*timer, iterations);
}


TestBoost::TestBoost()
  : BoostUT::test_suite("Boost timer variants")
{
  add(BOOST_TEST_CASE(threaded));
}


void TestBoost::threaded()
{ std::vector<boost::thread> threads;
  std::vector<unsigned> expected;
  std::vector<QuietThreadedTimer*> timers;
  const unsigned ntimers = 11, nthreads = 200;

  for (unsigned i=0; i<ntimers; ++i) {
    std::stringstream ident;
    ident << "Boost threads [" << i << "]";

    timers.push_back(new QuietThreadedTimer(ident.str()));
    expected.push_back(0);
  }

  for (unsigned i=0; i<nthreads; ++i) {
    unsigned slot = i % ntimers;
    QuietThreadedTimer *tmr = timers[slot];
    unsigned iterations = 200 + (std::rand() % 6101);

    threads.push_back(boost::thread(keepBusy, tmr, iterations));
    expected[slot] += iterations;
  }

  while (!threads.empty()) {
    threads.back().join();
    threads.pop_back();
  }

  for (unsigned i=0; i<ntimers; ++i) {
    BOOST_CHECK_EQUAL(expected[i], timers[i]->getStats().count);
    delete timers[i];
  }
}


  }   // namespace testing
}   // namespace rtimers
