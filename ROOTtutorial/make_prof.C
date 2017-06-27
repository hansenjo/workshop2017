#include "TProfile.h"
#include "TRandom.h"

TProfile* hp;

void make_prof()
{
  hp = new TProfile("hp","Profile of shifted Gaussian",160,-4.,4.);
  for( int i=0; i<100000; ++i ) {
    double x = gRandom->Gaus(0,1);
    double y = gRandom->Gaus(1,2);
    hp->Fill(x,y);
  }
}
