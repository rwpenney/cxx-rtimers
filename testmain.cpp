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

#include "core.hpp"

namespace BoostUT = boost::unit_test;


namespace rtimers {
  namespace testing {

struct TestVarianceStats : BoostUT::test_suite
{
  TestVarianceStats()
    : BoostUT::test_suite("resursive mean/variance computation")
  {
    add(BOOST_TEST_CASE(simple));
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
};


struct RTtestSuite : BoostUT::test_suite
{
  RTtestSuite()
    : BoostUT::test_suite("runtime timer tests")
  {
    add(new TestVarianceStats);
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
