# Sandbox around multiplicative persistence

This is a toy project inspired by the numberphile video "[What's special about 277777788888899?](https://www.youtube.com/watch?v=Wim9WJeDTHQ)".

The [Multiplicative Persistence](http://mathworld.wolfram.com/MultiplicativePersistence.html) of a number is defined as below:

> Multiply all the digits of a number n by each other, repeating with the product until a single digit is obtained. The number of steps required is known as the multiplicative persistence, and the final digit obtained is called the multiplicative digital root of n.

## Notes:

Of course, the multiplicative persistence depends on the numeric base. In base 10,
the maximum known multiplicative persistence is 11, which is obtained for the number 277777788888899.

However, it was not proved that this is the maximum multiplicative persistence in base 10 (although it is conjectured).

According to Wolfram Alpha :
> There is no number <10^(233) with multiplicative persistence  >11 (Carmody 2001; updating Wells 1986, p. 78)


## Intent of this project

The intent of this project is to be as fast as possible, and to be able to explore numbers with several hundred of digits as fast as possible (and of course to go beyond 10^(233)).

As mentioned in the [numberphile video](https://www.youtube.com/watch?v=Wim9WJeDTHQ)", it is possible to largely reduce the numbers to be searched with the following heuristics:
* The digits sequence shall be ordered from smallest to biggest: the order of the digits does not matter.
* With the following rules for the digits:
    * "0" : shall never appear
    * "1" : shall never appear
    * "2" : 1 max
    * "3" : 1 max
    * "4" : 1 max if there is no 2
    * "5" : shall never appear
    * "6" : shall never appear
    * "7" : as many as desired
    * "8" : as many as desired
    * "9" : as many as desired


## Current architecture

This project compiles *only* with clang >= 7 with coroutines enabled.

It uses :
* coroutines (-fcoroutines-ts)
* [conan](conan.io) in order to gather the third parties below
* [LoopPerfect/conduit](https://github.com/LoopPerfect/conduit) : High Performance Streams Based on Coroutine TS (see file conduit-unity.hpp)
* [gmplib](https://gmplib.org/): The GNU Multiple Precision Arithmetic Library
* [spdlog](https://github.com/gabime/spdlog): a Fast C++ logging library.
* [boost thread pool](https://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio/reference/thread_pool.html)


## Building and running the project

```bash
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

mkdir build
cd build
conan install .. --build=missing
cmake ..
make
./bin/persistence_coro
```

## Current status

`persistence_naive.cpp` is a naive implementation

`persistence_coro.cpp`is a more advanced implementation. It is quite fast when compared to the current record
mentioned on Wolfram Alphe : 10^233 is the record mentioned by Wolfram Alpha, but I suspect there are some
more recent and more advanced records.
)
On my computer, this program can compute
* up to 10^(233) in 7 minutes (using 16 cores)
* up to 10^(400) in 55 minutes (using 16 cores)

The performance is O(NbDigits^4) (see [graph](https://docs.google.com/spreadsheets/d/1gnz9wBsdIKxQY2LeKAtH20oGJYtueMJlGWT4QxQer4k/edit#gid=1408557870))

I stopped at 10^411 (after 1 hour), so that I can safely say that
> There is no number <10^(411) with multiplicative persistence > 11
