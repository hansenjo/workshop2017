#include "TH2F.h"
#include "TRandom.h"

TH2* h2D;

void make_2D()
{
  h2D = new TH2F("h2D","2D Gaussian peak",160,-4.,4.,160,-4.,4.);
  for( int i=0; i<100000; ++i ) {
    double x = gRandom->Gaus(0,1);
    double y = gRandom->Gaus(1,2);
    h2D->Fill(x,y);
  }
}
