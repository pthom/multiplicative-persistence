//#define ALGO_USE_VECTORS
#define ALGO_USE_COROUTINES
//#define ALGO_USE_RANGES

#include <iostream>
#include <sstream>
#include <vector>
#include <future>
#include <array>
#include <gmpxx.h>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio.hpp>
#include "spdlog/spdlog.h"

#ifdef ALGO_USE_COROUTINES
#include <experimental/coroutine>
#include "conduit-unity.hpp"
#endif

#ifdef ALGO_USE_RANGES
#include <range/v3/all.hpp>
using namespace ranges;
#endif

using BigInt = mpz_class;

BigInt gCurrentMaxPersistence = 0;
std::mutex gCurrentMaxMutex;

class stopwatch
{
public:
    stopwatch() : beg_(clock::now()) {}

    double elapsed() const {
        return std::chrono::duration_cast<second>
            (clock::now() - beg_).count(); }
private:
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::duration<double, std::ratio<1>> second;
    std::chrono::time_point<clock> beg_;
};

inline BigInt OneTransform(BigInt digits)
{
  // Note:
  // by making multiplied_digits and last_digit thread_local
  // we get a speedup factor of 2.5! (no more mallocs)
  thread_local BigInt multiplied_digits(1);
  multiplied_digits = 1;
  while(digits > 0)
  {
    thread_local BigInt last_digit;
    // the next line computes in one pass the equivalent of:
    //  "digits = digits / 10" and "last_digit = digits % 10"
    mpz_fdiv_qr_ui(digits.get_mpz_t(), last_digit.get_mpz_t(), digits.get_mpz_t(), 10);
    multiplied_digits = multiplied_digits * last_digit;
  }
  return multiplied_digits;
}

inline int PersistenceValue(BigInt v)
{
  int n = 0;
  while(v >= 10)
  {
    v = OneTransform(v);
    n++;
  }
  return n;
}

std::vector<int> numbers_between(int min, int max)
{
  std::vector<int> r;
  r.reserve(max - min);
  for (int i = min; i < max; i++)
    r.push_back(i);
  return r;
}

std::vector<int> numbers_up_to(int max)
{
  std::vector<int> r;
  r.reserve(max);
  for (int i = 0; i < max; i++)
    r.push_back(i);
  return r;
}

#if defined(ALGO_USE_VECTORS)
std::vector<std::array<int, 3>>
AllPossibleTripletsWithSum(int sum)
{
  std::vector<std::array<int, 3>> r;
  for (auto v1 : numbers_up_to(sum + 1))
    for (auto v2 : numbers_up_to(sum + 1 - v1))
      r.push_back(std::array<int, 3> { v1, v2, sum - v1 - v2 });
  return r;
}
#elif defined(ALGO_USE_COROUTINES)
conduit::seq<std::array<int, 3>>
AllPossibleTripletsWithSum(int sum)
{
  for (auto v1 : numbers_up_to(sum + 1))
    for (auto v2 : numbers_up_to(sum + 1 - v1))
      co_yield std::array<int, 3> { v1, v2, sum - v1 - v2 };
}
#elif defined(ALGO_USE_RANGES)
auto AllPossibleTripletsWithSum(int sum)
{
  return view::for_each(view::ints(0, sum + 1), [=](int v1) {
    return view::for_each(view::ints(0, sum + 1 - v1), [=](int v2) {
      return ranges::yield(std::array<int, 3> { v1, v2, sum - v1 - v2 });
    });
  });
}
#endif

inline BigInt DigitsToBigInt(const std::vector<int> & digits)
{
  char buffer[10000];
  auto len = digits.size();
  for (std::size_t i = 0; i < len; i++)
    buffer[i] = '0' + digits[i];
  buffer[len] = '\0';
  auto r = BigInt(buffer);
  return r;
}

std::string showDigits(std::vector<int> digits)
{
  std::stringstream ss;
  for (auto d: digits)
    ss << d;
  return ss.str();
}

// candidateNumbersWithNbDigits : returns a sequence
// of all the candidate numbers that shall be tested
// for a given number of digits
//
// The digits sequence shall be ordered from smallest to biggest
// with the following rules:
// "0" : None
// "1" : None
// "2" : 1 max
// "3" : 1 max
// "4" : 1 max if there is no 2
// "5" : None
// "6" : None
// "7" : as many as desired
// "8" : as many as desired
// "9" : as many as desired

#if ! defined(ALGO_USE_RANGES)

#if defined(ALGO_USE_VECTORS)
std::vector<BigInt> candidateNumbersWithNbDigits(int nbDigits)
#elif defined(ALGO_USE_COROUTINES)
conduit::seq<BigInt> candidateNumbersWithNbDigits(int nbDigits)
#endif
{
  #ifdef ALGO_USE_VECTORS
  std::vector<BigInt> result;
  #endif

  std::vector<int> digits(nbDigits, 5); // initialize all with 5 (5 is an incoherent value)
  static auto range_3 = std::vector<int> { 1, 0 };
  static auto range_2_4 = std::vector< std::pair<int, int> >
  {
    { 1, 0 },
    { 0, 1 },
    { 0, 0 }
  };

  for (auto nb_3: range_3)
  {
    for (auto v_2_4: range_2_4)
    {
      int nb_2 = v_2_4.first;
      int nb_4 = v_2_4.second;

      // Fill start of array with 2, 3, and 4 as needed
      int idx234 = 0;
      for (int i = 0; i < nb_2; i++) {
        digits[idx234] = 2;
        idx234++;
      }
      for (int i = 0; i < nb_3; i++) {
        digits[idx234] = 3;
        idx234++;
      }
      for (int i = 0; i < nb_4; i++) {
        digits[idx234] = 4;
        idx234++;
      }

      int nb_789 = nbDigits - nb_2 - nb_3 - nb_4;
      auto all_triplets_789 = AllPossibleTripletsWithSum(nb_789);
      for (auto triplet_789 : all_triplets_789 )
      {
        // digits 7 to 9 start after 234
        int idx789 =idx234;

        int nb_7 = triplet_789[2], nb_8 = triplet_789[1], nb_9 = triplet_789[0];
        for (int i = 0; i < nb_7; i++) {
          digits[idx789] = 7;
          idx789++;
        }
        for (int i = 0; i < nb_8; i++) {
          digits[idx789] = 8;
          idx789++;
        }
        for (int i = 0; i < nb_9; i++) {
          digits[idx789] = 9;
          idx789++;
        }
        auto bigInt = DigitsToBigInt(digits);

        #ifdef ALGO_USE_VECTORS
        result.push_back(bigInt);
        #endif
        #ifdef ALGO_USE_COROUTINES
        co_yield bigInt;
        #endif
      }
    }
  }

  #ifdef ALGO_USE_VECTORS
  return result;
  #endif
}

#elif defined(ALGO_USE_RANGES)

auto range_3 = std::vector<int> { 1, 0 };
auto range_2_4 = std::vector< std::pair<int, int> >
{
  { 1, 0 },
  { 0, 1 },
  { 0, 0 }
};

auto candidateNumbersWithNbDigits(int nbDigits)
{
  return view::for_each(range_3, [=](int nb_3)
  {
    return view::for_each(range_2_4, [=](std::pair<int, int> v_2_4)
    {
      int nb_2 = v_2_4.first;
      int nb_4 = v_2_4.second;

      int nb_789 = nbDigits - nb_2 - nb_3 - nb_4;

      return view::for_each(AllPossibleTripletsWithSum(nb_789), [=](auto triplet_789)
      {
        //return ranges::yield(BigInt(3));
        std::vector<int> digits(nbDigits, 5); // initialize all with 5 (5 is an incoherent value)
        // Fill start of array with 2, 3, and 4 as needed
        int idx234 = 0;
        for (int i = 0; i < nb_2; i++) {
          digits[idx234] = 2;
          idx234++;
        }
        for (int i = 0; i < nb_3; i++) {
          digits[idx234] = 3;
          idx234++;
        }
        for (int i = 0; i < nb_4; i++) {
          digits[idx234] = 4;
          idx234++;
        }

        // digits 7 to 9 start after 234
        int idx789 =idx234;

        int nb_7 = triplet_789[2], nb_8 = triplet_789[1], nb_9 = triplet_789[0];
        for (int i = 0; i < nb_7; i++) {
          digits[idx789] = 7;
          idx789++;
        }
        for (int i = 0; i < nb_8; i++) {
          digits[idx789] = 8;
          idx789++;
        }
        for (int i = 0; i < nb_9; i++) {
          digits[idx789] = 9;
          idx789++;
        }
        auto bigInt = DigitsToBigInt(digits);
        return ranges::yield(bigInt);
      });
    });
  });
}
#endif // #elif defined(ALGO_USE_RANGES)


inline int TestOneNumber(BigInt number)
{
  int persistence = PersistenceValue(number);
  bool isNewMax = [&]() {
    std::lock_guard lock(gCurrentMaxMutex);
    if (persistence <= gCurrentMaxPersistence)
      return false;
    else {
      gCurrentMaxPersistence = persistence;
      return true;
    }
  }();
  if (isNewMax)
  {
    spdlog::warn("New max at {} with persistence={}", number.get_str(), persistence);
    gCurrentMaxPersistence = persistence;
  }
  return persistence;
}

// Checks the conjecture :
// all maximum persistence number > 10^200
// are under the form 237777777....
bool checkConjecture237(BigInt n)
{
  std::string s = n.get_str();
  if (s[0] != '2')
    return false;
  if (s[1] != '3')
    return false;
  for (std::size_t i = 2; i < s.size(); i++)
    if (s[i] != '7')
      return false;
  return true;
}

void playground()
{
    // std::cout << PersistenceValue(BigInt("277777788888899")).get_str() << std::endl;
  // return 0;

  // BigInt n("4553435645654334326577686587487773537637376387367676765753756664357452435234523534343553265654654437645657474777737"), d(10);
  // BigInt quotient(0), remainder(0);
  // mpz_fdiv_qr_ui (quotient.get_mpz_t(), remainder.get_mpz_t(), n.get_mpz_t(), 10);
  // std::cout << "n=" << n.get_str() << std::endl
  //   << "q=" << quotient.get_str() << " r=" << remainder.get_str() << "\n";
  // return 0;

  // BigInt n("4553435645654334326577686587487773537637376387367676765753756664357452435234523534343553265654654437645657474777737"), d(10);
  // BigInt quotient = n / 10;
  // BigInt remainder = n % 10;
  // std::cout << "n=" << n.get_str() << " d=" << d.get_str()
  //   << " quotient=" << quotient.get_str() << " remainder=" << remainder.get_str() << "\n";
  // return 0;




  // for (auto v: candidateNumbersWithNbDigits(4))
  //   std::cout << v.get_str()<< "\n";

  // for (auto v: AllPossibleTripletsWithSum(4))
  //   std::cout << v[0] << "," << v[1] << "," << v[2] << "\n";
}

void process_for_nb_digits(int nb_digits)
{
  spdlog::info("Starting nb_digits={}", nb_digits);
  stopwatch timer;
  int max_persistence_this_loop = -1;
  BigInt record_holder(0);
  for (auto number: candidateNumbersWithNbDigits(nb_digits))
  {
    int persistence = TestOneNumber(number);
    if (persistence > max_persistence_this_loop) {
      max_persistence_this_loop = persistence;
      record_holder = number;
    }
  }
  spdlog::info("Finished nb_digits={}\n"
    "nb_digits,time,max_persistence,where:{},{},{},{}",
    nb_digits,
    nb_digits, timer.elapsed(), max_persistence_this_loop, record_holder.get_str()
  );
  bool conjecture_test = checkConjecture237(record_holder);
  if (!conjecture_test)
  {
    spdlog::warn("Conjecture not verified for nb_digits={} persistence={} for {}\n",
      nb_digits, max_persistence_this_loop, record_holder.get_str() );
  }
}


int main()
{
  // playground(); return 0;

  // Launch the search inside a pool thread
  int nb_cores = 16;
  boost::asio::thread_pool pool(nb_cores);
  for (auto nb_digits : numbers_between(4, 2000))
  {
    boost::asio::post(pool, [nb_digits]() {
      process_for_nb_digits(nb_digits);
    });
  }
  pool.join();
}
