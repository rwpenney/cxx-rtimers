#include <iomanip>
#include <rtimers/cxx11.hpp>

using namespace rtimers;


void expensiveFunction() {
  static rtimers::cxx11::DefaultTimer timer("expensive");
  auto scopedStartStop = timer.scopedStart();
}


int main(int argc, char* argv[])
{
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
