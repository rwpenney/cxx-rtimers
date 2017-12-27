/*
 *  POSIX unit-tests for run-time timers
 */

//  Copyright (C) 2017-2018, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <boost/test/unit_test.hpp>
#include <cmath>

#include "testdefns.hpp"

#if RTIMERS_HAVE_POSIX
#  include <pthread.h>
#  include "rtimers/posix.hpp"
#endif

namespace BoostUT = boost::unit_test;


namespace rtimers {
  namespace testing {

#if RTIMERS_HAVE_POSIX

using QuietSerialTimer = Timer<SerialManager<posix::HiResClock,
                                             MeanBoundStats>,
                               NullLogger>;
using QuietThreadedTimer = Timer<posix::ThreadManager<posix::HiResClock,
                                                      MeanBoundStats>,
                                 NullLogger>;

struct BusyParams {
  QuietThreadedTimer* timer;
  unsigned iterations;
};

static void* keepBusy(void* arg)
{ BusyParams* params = static_cast<BusyParams*>(arg);
  double tot = 0.0;

  for (unsigned n=0; n<params->iterations; ++n) {
    params->timer->start();
    tot += std::cos((n * 252 + 23) % 41);
    params->timer->stop();
  }

  return NULL;
}

#endif  // RTIMERS_HAVE_POSIX


TestPosix::TestPosix()
  : BoostUT::test_suite("POSIX timer variants")
{
  add(BOOST_TEST_CASE(serial));
  add(BOOST_TEST_CASE(threaded));
}


void TestPosix::serial()
{
#if RTIMERS_HAVE_POSIX
  QuietSerialTimer tmr("C++ clock_gettime");
  double tot = 0.0;
  const unsigned iterations = 150;

  for (unsigned i=0; i<iterations; ++i) {
    tmr.start();
    tot += std::exp(std::sin(i * 2.1));
    // Assume that this takes at least 20ns, including zero offset
    tmr.stop();
  }
  BOOST_CHECK(tot != 0.0);

  const auto stats = tmr.getStats();
  BOOST_CHECK_EQUAL(stats.count, iterations);
  BOOST_CHECK_GT(stats.mean, 20e-9);
  BOOST_CHECK_LT(stats.tmax, 1.0);
#else // !RTIMERS_HAVE_POSIX
  BOOST_ERROR("No clock_gettime support");
#endif  // RTIMERS_HAVE_POSIX
}


void TestPosix::threaded()
{
#if RTIMERS_HAVE_POSIX
  QuietThreadedTimer tmr("POSIX threads");
  std::list<pthread_t> threads;
  std::vector<BusyParams*> params;
  const unsigned nthreads = 200;

  for (unsigned i=0; i<nthreads; ++i) {
    pthread_t handle;

    BusyParams *args = new BusyParams;
    args->timer = &tmr;
    args->iterations = 2 * i;
    params.push_back(args);

    pthread_create(&handle, NULL, keepBusy, (void*)args);
    threads.push_back(handle);
  }

  while (!threads.empty()) {
    pthread_join(threads.back(), NULL);
    threads.pop_back();

    delete params.back();
    params.pop_back();
  }

  BOOST_CHECK_EQUAL(tmr.getStats().count, nthreads * (nthreads - 1));
#else // !RTIMERS_HAVE_POSIX
  BOOST_ERROR("No POSIX multi-thread support");
#endif  // RTIMERS_HAVE_POSIX
}


  }   // namespace testing
}   // namespace rtimers
