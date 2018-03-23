#include <iostream>
#include "mtrand/mtrand.h"
#include "nheader.h"
#include <time.h>

using namespace std;

MTRand mt(time(NULL));

void mtinit(int * seed)
{
  if (*seed == 0) {
    *seed = time(NULL);
  }
  mt.seed(*seed);
}

double mtrand()
{
  return mt();
}
