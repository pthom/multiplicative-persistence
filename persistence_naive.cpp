#include <iostream>
#include <vector>
#include <atomic>
#include <future>
#include <gmpxx.h>

// https://www.youtube.com/watch?v=Wim9WJeDTHQ

using Int = long long;
using Ints = std::vector<Int>;



inline Int OneTransform(Int v)
{
  Int current = v;
  Int r = current % 10;
  while(current > 10) {
    current = current / 10;
    r = r * (current % 10);
  }
  return r;
}

inline Int PersistenceValue(Int v)
{
  Int n = 0;
  while(v >= 10)
  {
    v = OneTransform(v);
    n++;
  }
  return n;
}

void LogProgress(Int start, Int end, Int i)
{
  if (i % 100000000 == 0)
  {
    //double percent = (double) i / (double)(end - start );
    //std::cout << "Percent: " << percent * 100. << "\n";
  }
}

Int currentMax;

void search(Int start, Int end, int thread_id)
{
  for (Int i = start; i < end; i++)
  {
    if (thread_id == 0)
      LogProgress(start, end, i);
    Int r = PersistenceValue(i);
    if (r > currentMax)
    {
      currentMax = r;
      std::cout
        << "New max at i=" << i.get_str()
        << " => " << currentMax.get_str()
        << " (thread " << thread_id << ")" << "\n";
    }
  }
}

Int proposeNumber()
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

  // donner les chiffres dans l'ordre croissant
  return 0;
}


int main()
{
  currentMax = 0;
  Int start = 277777788888899 - 10;
  //Int end = 100000000000;
  Int end = start * 2;
  int nb_threads = 8;

  std::vector< std::future<void> > asyncResults;
  for (int i = 0; i < nb_threads; i++)
  {
    asyncResults.push_back( std::async(
      std::launch::async,
      [=](){
        Int c_start = start + (end - start) / nb_threads * i;
        Int c_end = start + (end - start) / nb_threads * (i + 1);
        search(c_start, c_end, i);
      }) );
  }

}