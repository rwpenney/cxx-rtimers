#include "chrono.hpp"

using namespace rtimers;


int main(int argc, char* argv[])
{
  typedef Timer<SerialManager<chrono::HiResClock, VarBoundStats>,
                StderrLogger> BasicTimer;
  typedef Timer<NullManager, NullLogger> NullTimer;

  { BasicTimer tmr("bare");

    for (int i=0; i<10; ++i) {
      tmr.start();
      tmr.stop();
    }
  }

  { BasicTimer tmr("auto");

    for (int i=0; i<2000; ++i) {
      BasicTimer::Scoper sc = tmr.scopedStart();
      // Do heavy computation...
    }
  }

  { NullTimer tmr("null");

    for (int i=0; i<1000; ++i) {
      tmr.start();
      tmr.stop();
    }
  }

  return 0;
}
