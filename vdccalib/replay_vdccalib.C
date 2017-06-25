/* VDC TDC calibration replay 
   Jens-Ole Hansen  ole@jlab.org  24-Jun-2010
   Requires analyzer 1.5.15 or higher

   Updated for ROOT 6 and analyzer 1.6  24-June-2017
*/

#include <TString.h>
#include <TSystem.h>
#include <TFile.h>
#include <iostream>
#include "THaAnalyzer.h"
#include "THaHRS.h"
#include "THaVDC.h"
#include "THaRun.h"
#include "THaGlobals.h"

using namespace std;

TString getrun( Int_t& irun );
Int_t getnev();

//-------------------------------------------------------------------------
void replay_vdccalib( const char* arm="R", Int_t runnumber=-1) {

  TString spec(arm);
  spec.ToUpper();
  if( spec != "L" && spec != "R" ) {
    cout << "usage: replay_vdccalib(\"L\"/\"R\", runnumber)" << endl;
    return;
  }

  gHaTextvars->Set("arm",arm);

  TString codafile = getrun( runnumber );
  if( codafile.IsNull() ) return;
  if( runnumber < 0 ) return;
  nev = getnev();

  THaAnalyzer* analyzer = THaAnalyzer::GetInstance();
  if( analyzer ) {
    analyzer->Close();
  } else {
    analyzer = new THaAnalyzer;
  }

  // Set up the HRS spectrometer
  TString dscrpt = (spec == "L" ? "Left" : "Right");
  dscrpt.Append(" HRS");
  THaHRS* hrs = new THaHRS(spec, dscrpt);
  THaDetector* vdc = hrs->GetDetector("vdc");
  if( !vdc ) {
    vdc = new THaVDC("vdc", Form("%s HRS VDCs",dscrpt.Data()));
    hrs->AddDetector(vdc);
  }
  vdc->SetBit(THaVDC::kDecodeOnly);   // Decode only, don't track
  gHaApps->Add(hrs);

  TString rootfile(Form("rootfiles/vdccalib_%d.root",runnumber));
  analyzer->SetOutFile(rootfile);
  analyzer->SetOdefFile("replay_vdccalib.odef");
  analyzer->SetCutFile("replay_vdccalib.cdef");
  analyzer->SetMarkInterval(5000);
  analyzer->SetCompressionLevel(5);
  analyzer->EnableBenchmarks(true);

  THaRun* run = new THaRun(codafile);
  run->SetLastEvent(nev);

  // Action
  analyzer->Process(run);

  analyzer->Close();
  delete run; run = 0;
  delete analyzer; analyzer = 0;
  gHaVars->Delete();
  gHaApps->Delete();

  new TFile(rootfile,"r");
}

//-------------------------------------------------------------------------
TString getrun( Int_t& irun )
{
  // Get the run to analyze
  const char* paths[] = {
    "data/gmp",
    "data/Ar40",
    "data/g2p",
    0
  };

  // Define the analysis parameters
  if(irun<0){
    cout << "Run number (-1 to quit)? ";
    cin >> irun;
  }
  if( irun<0 )
    return TString();

  TString filename;
  bool found = false;
  const char** path = paths;
  while ( path && *path ) {         
    filename = Form("%s_%d.dat.0",*path,irun);
    gSystem->ExpandPathName(filename);
    if( !gSystem->AccessPathName( filename, kReadPermission )) {
      found = true;
      cout << "Will analyze file " << filename << endl;
      break;
    }
    path++;
  }
  if ( (!path || !*path) && !found ) {
    cout << "File not found.  Try again, or -1 to quit" << endl;
    filename = "";
  }
  return filename;
}

//-------------------------------------------------------------------------
Int_t getnev() {
  int nev;
  cout << "Number of events to analyze? (-1 for all)? ";
  cin >> nev;
  return nev;
}
