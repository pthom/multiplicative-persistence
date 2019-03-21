#include <iostream>
#include <sstream>
#include <vector>
#include <atomic>
#include <future>
#include <array>
#include <gmpxx.h>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio.hpp>
#include <experimental/coroutine>
#include "conduit-unity.hpp"

// https://www.youtube.com/watch?v=Wim9WJeDTHQ

using BigInt = mpz_class;

inline BigInt OneTransform(BigInt v)
{
  BigInt current = v;
  BigInt r = current % 10;
  while(current > 10) {
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
  std::stringstream ss;
  for (auto d : digits) {
    char c = '0' + d;
    ss << c;
  }
  return BigInt( ss.str() );
}

std::string showDigits(std::vector<int> digits)
{
  std::stringstream ss;
  for (auto d: digits)
    ss << d;
  return ss.str();
}

conduit::seq<BigInt> testedNumbersSequences(int nbDigits)
{
  // 0 : Aucun
  // 1 : Aucun
  // 2 : 1 max
  // 3 : 1 max
  // 4 : 1 max si pas de 2
  // 5 : Aucun
  // 6 : Aucun
  // 7 : plein
  // 8 : Plein
  // 9 : Plein

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

      //std::cout << "nb_2=" << nb_2 << " nb_3=" << nb_3 << " nb_4=" << nb_4 << " digits=" << showDigits(digits) << std::endl;


      int nb_789 = nbDigits - nb_2 - nb_3 - nb_4;
      //std::cout << "nb_789=" << nb_789 << std::endl;

      //std::cout << "nb_2=" << nb_2 << " nb_3=" << nb_3 << " nb_4=" << nb_4 << " nb_789=" << nb_789 << std::endl;

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

BigInt currentMax;

int main()
{

  auto process_for_nb_digits = [](int nb_digits) {
    std::cout << "nb_digits=" << nb_digits << std::endl;
    for (auto number: testedNumbersSequences(nb_digits))
    {
      auto persistence = PersistenceValue(number);
      if (persistence > currentMax)
      {
        std::cout << "New max at " << number.get_str() << " with persistence=" << persistence.get_str() << std::endl;
        currentMax = persistence;
      }
    }
  };


  // Launch the pool with four threads.
  int nb_cores = 16;
  boost::asio::thread_pool pool(nb_cores);

  for (auto nb_digits : numbers_between(4, 600))
  {
    boost::asio::post(pool, [nb_digits, &process_for_nb_digits]() {
      process_for_nb_digits(nb_digits);
    });
  }
  pool.join();

  //for (auto v: testedNumbersSequences(20))
    //std::cout << v.get_str()<< "\n";

  //for (auto v: AllPossibleTripletsWithSum(1))
    //std::cout << v[0] << "," << v[1] << "," << v[2] << "\n";

}
