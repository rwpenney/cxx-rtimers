/*
 *  Unit-tests for run-time timer mechanisms
 */

//  Copyright (C) 2017, RW Penney

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>

#include "rtimers/core.hpp"
#include "testdefns.hpp"

namespace BoostUT = boost::unit_test;


namespace rtimers {
  namespace testing {


struct TestStartStop : BoostUT::test_suite
{
  typedef Timer<SerialManager<C89clock, BoundStats>, NullLogger> QuietTimer;

  TestStartStop()
    : BoostUT::test_suite("timer start/stop counting")
  {
    add(BOOST_TEST_CASE(plain));
    add(BOOST_TEST_CASE(scoped));
  }

  static void plain() {
    QuietTimer tmr("basic");
    const unsigned count = 7831;

    for (unsigned i=0; i<count; ++i) {
      tmr.start();
      tmr.stop();
    }

    BOOST_CHECK_EQUAL(tmr.getStats().count, count);
  }

  static void scoped() {
    QuietTimer tmr("basic");
    const unsigned count = 1384;

    for (unsigned i=0; i<count; ++i) {
      QuietTimer::Scoper sc = tmr.scopedStart();
    }

    BOOST_CHECK_EQUAL(tmr.getStats().count, count);
  }
};


/** Inject sequence of samples with well-known mean and variance */
template <typename STATS>
void pushSineSamples(STATS& stats, unsigned count, double offset, double amp)
{
  for (unsigned i=0; i<count; ++i) {
    const double x = i / (double)count;
    stats.addSample(offset + amp * std::sin(8 * Pi * x));
  }
}


struct TestVarianceStats : BoostUT::test_suite
{
  TestVarianceStats()
    : BoostUT::test_suite("resursive mean/variance computation")
  {
    add(BOOST_TEST_CASE(simple));
    add(BOOST_TEST_CASE(sine));
  }

  static void simple() {
    VarBoundStats stats;
    const double eps = 1e-9;

    stats.addSample(1);
    stats.addSample(3);
    stats.addSample(4);
    stats.addSample(2);

    BOOST_CHECK_EQUAL(stats.count, 4);

    BOOST_CHECK_EQUAL(stats.tmin, 1.0);
    BOOST_CHECK_EQUAL(stats.tmax, 4.0);

    BOOST_CHECK_CLOSE(stats.mean, 2.5, eps);

    BOOST_CHECK_CLOSE(stats.nVariance, 2 * (0.25 + 2.25), eps);
    BOOST_CHECK_CLOSE(stats.getStddev(), std::sqrt((0.25 + 2.25) / 2), eps);
  }

  static void sine() {
    VarBoundStats stats;
    const double mean = 16.5, amp = 2.3, eps = 1e-3;
    const unsigned count = 10000;

    pushSineSamples(stats, count, mean, amp);

    BOOST_CHECK_EQUAL(stats.count, count);

    BOOST_CHECK_CLOSE(stats.tmin, mean - amp, eps);
    BOOST_CHECK_CLOSE(stats.tmax, mean + amp, eps);

    BOOST_CHECK_CLOSE(stats.mean, mean, eps);

    BOOST_CHECK_CLOSE(stats.nVariance, count * 0.5 * amp * amp, eps);
    BOOST_CHECK_CLOSE(stats.getStddev(), std::sqrt(0.5) * amp, eps);
  }
};


struct RTtestSuite : BoostUT::test_suite
{
  RTtestSuite()
    : BoostUT::test_suite("runtime timer tests")
  {
    add(new TestStartStop);
    add(new TestVarianceStats);

    add(new TestCxx11);
  }
};


//! Unit-test initialization mechanism for boost::unit_test
bool init_unit_test_suite()
{
  BoostUT::framework::master_test_suite().add(new RTtestSuite);
  return true;
}

  }   // namespace testing
}   // namespace rtimers


int main(int argc, char *argv[])
{
  return BoostUT::unit_test_main(
          (BoostUT::init_unit_test_func)rtimers::testing::init_unit_test_suite,
          argc, argv);
}
