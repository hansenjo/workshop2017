#include "TH1I.h"
#include "TRandom.h"

TH1 *h1, *h2;

void make_histos() {
  h1 = new TH1I("h1","Gaussian distribution",160,-4.,4.);
  h2 = new TH1I("h2","Shifted narrower Gaussian",160,-4.,4.);
  for ( int i=0; i<10000; ++i ) {
    double x;
    x = gRandom->Gaus(0,1);
    h1->Fill(x);
    x = gRandom->Gaus(0.2,0.8);
    h2->Fill(x);
  }
}
