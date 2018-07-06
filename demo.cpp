/*
 *  Examples of C++ stopwatch classes, based around
 *    https://github.com/rwpenney/cxx-rtimers/tree/master/rtimers
 *  NOTE: this application requires a compiler with C++11 support (or later)
 */
#include <cmath>
#include <iomanip>
#include <rtimers/cxx11.hpp>
#if RTIMERS_HAVE_BOOST
#  include <rtimers/boost.hpp>
#endif
#if defined(__linux)
#  include <rtimers/posix.hpp>
#endif

using namespace rtimers;


double expensiveFunction() {
  static rtimers::cxx11::DefaultTimer timer("expensive");
  auto scopedStartStop = timer.scopedStart();
  double result = 0.0;

  for (int i=0; i<100; ++i) {
    result += std::cos(0.2 * i + 0.1);
  }

  return result;
}


int main(int argc, char* argv[])
{
  // Estimate zero-offset on available clocks
  //  (this may well be at least 100ns):
  std::cout << "Zero errors:" << std::endl
      << "  default: "
      << cxx11::DefaultTimer::zeroError<MeanBoundStats>() << std::endl
#if RTIMERS_HAVE_BOOST
      << "  Boost: "
      << clockZeroError<rtimers::boostpt::HiResClock, MeanBoundStats>() << std::endl
#endif
#if defined(__linux)
      << "  POSIX: "
      << clockZeroError<rtimers::posix::HiResClock, MeanBoundStats>() << std::endl
#endif
      << std::endl;


  for (int i=0; i<731; ++i) {
    expensiveFunction();
  }

  { cxx11::DefaultTimer tmr("bare");

    for (int i=0; i<10; ++i) {
      tmr.start();
      tmr.stop();
    }
  }

  { cxx11::ThreadedTimer tmr("auto");

    for (int i=0; i<2000; ++i) {
      auto sc = tmr.scopedStart();
      // Do heavy computation...
    }
  }

  { NullTimer tmr("null");

    for (int i=0; i<1000; ++i) {
      tmr.start();
      tmr.stop();
    }
  }

  // Prepare number formatting for timer output on program exit:
  std::cerr << std::fixed << std::setprecision(3);

  return 0;
}
