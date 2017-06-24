#include "THaAnalyzer.h"
#include "THaHRS.h"
#include "THaVDC.h"
#include "THaScintillator.h"
#include "THaCherenkov.h"
#include "THaRun.h"
#include "THaGlobals.h"

using namespace std;

THaRun* run;
THaAnalyzer* analyzer;

void toy_setup() {
  auto RHRS = new THaHRS("R","Right HRS");
  RHRS->AddDetector( new THaVDC("vdc","RHRS VDC"));
  RHRS->AddDetector( new THaScintillator("s0","RHRS S0"));
  RHRS->AddDetector( new THaScintillator("s2","RHRS S2m"));
  RHRS->AddDetector( new THaCherenkov("cer","RHRS Ckov"));
  gHaApps->Add(RHRS);
  analyzer = new THaAnalyzer;
  run = new THaRun("/data/raw/gmp_23062.dat.0");
  run->SetLastEvent(10000);
  analyzer->SetOutFile("rootfiles/junk.root");
  analyzer->EnableBenchmarks();
}
