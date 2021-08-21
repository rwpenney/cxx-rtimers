# C++ run-time timers

This is a set of C++ stopwatch classes for gathering
statistical data on the run-time performance of client code.
The classes will accumulate data on the time taken
for each start/stop interval, and report parameters
such as the minimum, maximum and average time
after the timer goes out of scope.

Features:
* Header only
* Templated to allow customization of clock source, timing statistics, etc.
* C++03 and C++11 (std::chrono) support
* Scope-based or manual starting/stopping
* Thread-safe and serial options
* Numerically stable calculation of mean & standard-deviation statistics
* Automatic scaling to nanosecond/microsecond/minute/etc. units


## Example usage

```cpp
#include <rtimers/cxx11.hpp>

void expensiveFunction() {
    static rtimers::cxx11::DefaultTimer timer("expensive");
    auto scopedStartStop = timer.scopedStart();
    // Do something costly...
}
```

When the program terminates, the timer will send a report
of the following form to std:cerr
```
Timer(expensive): <t> = 8.867us, std = 3.463us, 4.263us <= t <= 57.62us (n=731)
```
which shows the average time spent within expensiveFunction(),
its standard deviation, the upper and lower bounds,
and the total number of calls.

On POSIX systems with older versions of C++, one could use:
```cpp
#include <rtimers/posix.hpp>
rtimers::posix::DefaultTimer timer("bottleneck");
```
Or for systems on which the [Boost](http://www.boost.org/)
libraries are available, including
the [boost::posix_time](http://www.boost.org/doc/libs/1_77_0/doc/html/date_time/posix_time.html) datastructures:
```cpp
#include <rtimers/boost.hpp>
rtimers::boostpt::DefaultTimer timer("bottleneck");
```

For multi-threaded code, one can use the following timer classes:
```cpp
rtimers::cxx11::ThreadedTimer
rtimers::posix::ThreadedTimer
rtimers::boostpt::ThreadedTimer
```

Preprocessor macros are available for the combined declaration
of a `static` timer instance and a scoped start+stop:
```cpp
RTIMERS_BOOST_STATIC_SCOPED(name)
RTIMERS_CXX11_STATIC_SCOPED(name)
RTIMERS_POSIX_STATIC_SCOPED(name)
RTIMERS_STATIC_SCOPED(name)
```

More specialized timers can be built by combining components
such as `rtimers::cxx11::HiResClock`, `rtimers::SerialManager`,
`rtimers::MeanBoundStats`, `rtimers::StreamLogger`, etc.
