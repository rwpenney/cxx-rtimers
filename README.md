# C++ run-time timers

This is a set of C++ stopwatch classes for gathering
statistics on the run-time performance of client code.
The classes will accumulate data on the time taken
for each start/stop interval, and report parameters
such as the minimum, maximum and average time
after the timer goes out of scope.


## Example usage

```cpp
#include <rtimers/cxx11.hpp>

void expensiveFunction() {
    static rtimers::cxx11::DefaultTimer timer("expensive");
    auto scopedStartStop = timer.scopedStart();
}
```

When the program terminates, the timer will send a report
of the following form to std:cerr
```
Timer(expensive): <t> = 1.469e-07, std = 2.1792e-08, 1.34e-07 <= t <= 2.01e-07 (n=731)
```
which shows the average time (in seconds) spent within expensiveFunction(),
its standard deviation, the upper and lower bounds,
and the total number of calls.
