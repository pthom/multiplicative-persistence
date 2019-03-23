#include <chrono>

class stopwatch
{
public:
    inline stopwatch() : beg_(clock::now()) {}
    inline double elapsed() const {
        return std::chrono::duration_cast<second>
            (clock::now() - beg_).count(); }
private:
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::duration<double, std::ratio<1>> second;
    std::chrono::time_point<clock> beg_;
};
