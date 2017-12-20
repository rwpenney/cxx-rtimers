#include "core.hpp"

using namespace rwputils;


int main(int argc, char* argv[])
{
  typedef Timer<SerialManager<C89clock, VarBoundStats>,
                StdoutLogger> BasicTimer;
  typedef Timer<NullManager, NullLogger> NullTimer;

  { BasicTimer tmr("bare");
    for (int i=0; i<10; ++i) {
      tmr.start();
      tmr.stop();
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
