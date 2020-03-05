#include <unistd.h>
#include <chrono>
#include <iostream>

class Timer
{
public:
    Timer(void) {}
    ~Timer(void) {}

public:
    void start() { time_start = std::chrono::steady_clock::now(); }
    void stop()
    {
        time_stop = std::chrono::steady_clock::now();
        time_used = std::chrono::duration_cast<std::chrono::duration<double>>(time_stop - time_start);
    }
    double milliseconds()
    {
        return time_used.count() * 1000;
    }
    double microseconds() { return time_used.count() * 1000 * 1000; }
    double seconds() { return time_used.count(); }

private:
    std::chrono::steady_clock::time_point time_start, time_stop;
    std::chrono::duration<double> time_used;
};