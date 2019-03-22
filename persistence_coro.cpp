#include <iostream>
#include <sstream>
#include <vector>
#include <future>
#include <array>
#include <gmpxx.h>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio.hpp>
#include "spdlog/spdlog.h"
#include <experimental/coroutine>
#include "conduit-unity.hpp"

using BigInt = mpz_class;
BigInt currentMax = 0;
std::mutex currentMax_Mutex;

inline BigInt OneTransform(BigInt v)
{
  thread_local BigInt current, r;
  current = v;
  r = current % 10;
  while(current > 10)
  {
    // WIP / Optim (quotient and remainder in one pass)
    // --------
    // thread_local BigInt quotient, remainder;
    // thread_local const auto ten = BigInt(10);
    // mpz_cdiv_qr (quotient.get_mpz_t(), remainder.get_mpz_t(), current.get_mpz_t(), ten.get_mpz_t());
    // current = quotient;
    // r = r * remainder;

    current = current / 10;
    r = r * (current % 10);
  }
  return r;
}

inline BigInt PersistenceValue(BigInt v)
{
  BigInt n = 0;
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

conduit::seq<std::array<int, 3>>
AllPossibleTripletsWithSum(int sum)
{
  for (auto v1 : numbers_up_to(sum + 1))
    for (auto v2 : numbers_up_to(sum + 1 - v1))
      co_yield std::array<int, 3> { v1, v2, sum - v1 - v2 };
}

inline BigInt DigitsToBigInt(const std::vector<int> & digits)
{
  thread_local char buffer[10000];
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

// candidateNumbersWithbNDigits : returns a sequence
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
conduit::seq<BigInt> candidateNumbersWithbNDigits(int nbDigits)
{
  std::vector<int> digits(nbDigits, 5); // initialize all with 5 (5 is an incoherent value)
  auto range_3 = std::vector<int> { 1, 0 };
  auto range_2_4 = std::vector< std::pair<int, int> >
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
        co_yield bigInt;
      }
    }
  }
}


int main()
{
  auto process_for_nb_digits = [](int nb_digits) {
    spdlog::info("Processing nb_digits={}", nb_digits);
    for (auto number: candidateNumbersWithbNDigits(nb_digits))
    {
      auto persistence = PersistenceValue(number);
      bool isNewMax = [&]() {
        std::lock_guard lock(currentMax_Mutex);
        if (persistence <= currentMax)
          return false;
        else {
          currentMax = persistence;
          return true;
        }
      }();
      if (isNewMax)
      {
        spdlog::warn("New max at {} with persistence={}", number.get_str(), persistence.get_str());
        currentMax = persistence;
      }
    }
  };

  // Launch the search inside a pool thread
  int nb_cores = 16;
  boost::asio::thread_pool pool(nb_cores);
  for (auto nb_digits : numbers_between(4, 2000))
  {
    boost::asio::post(pool, [nb_digits, &process_for_nb_digits]() {
      process_for_nb_digits(nb_digits);
    });
  }
  pool.join();

  //for (auto v: candidateNumbersWithbNDigits(20))
    //std::cout << v.get_str()<< "\n";

  //for (auto v: AllPossibleTripletsWithSum(1))
    //std::cout << v[0] << "," << v[1] << "," << v[2] << "\n";

}
