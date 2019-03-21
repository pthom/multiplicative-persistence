#include <iostream>
#include <vector>
#include <atomic>
#include <future>

using BigInt = long long;

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

void LogProgress(BigInt start, BigInt end, BigInt i)
{
  if (i % 100000000 == 0)
  {
    double percent = (long double) i / (long double)(end - start );
    std::cout << "Percent: " << percent * 100. << "\n";
  }
}

BigInt currentMax;

void search(BigInt start, BigInt end, int thread_id)
{
  for (BigInt i = start; i < end; i++)
  {
    if (thread_id == 0)
      LogProgress(start, end, i);
    BigInt r = PersistenceValue(i);
    if (r > currentMax)
    {
      currentMax = r;
      std::cout
        << "New max at i=" << i
        << " => " << currentMax
        << " (thread " << thread_id << ")" << "\n";
    }
  }
}

int main()
{
  currentMax = 0;
  BigInt start = 277777788888899 - 10;
  //BigInt end = 100000000000;
  BigInt end = start * 2;
  int nb_threads = 8;

  std::vector< std::future<void> > asyncResults;
  for (int i = 0; i < nb_threads; i++)
  {
    asyncResults.push_back( std::async(
      std::launch::async,
      [=](){
        BigInt c_start = start + (end - start) / nb_threads * i;
        BigInt c_end = start + (end - start) / nb_threads * (i + 1);
        search(c_start, c_end, i);
      }) );
  }
}
