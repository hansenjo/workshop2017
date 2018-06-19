// VDC comparison replay for GMP12
// Must load OldVDC.so library before running
//
// Ole Hansen, 25 Jan 2018

#include <string>
#include <sstream>
#include "THaRun.h"
#include "THaAnalyzer.h"
#include "THaGlobals.h"
#include <iostream>
#include "TSystem.h"
#include "TError.h"
#include "TFile.h"
#include "THaTextvars.h"
#include "THaHRS.h"
#include "THaVDC.h"
#include "THaScintillator.h"
#include "THaCherenkov.h"
#include "THaIdealBeam.h"
#include "THaGoldenTrack.h"
#include "THaPrimaryKine.h"
#include "THaReactionPoint.h"
#include "THaVarList.h"
#include "THaCutList.h"

using namespace std;

struct Experiment {
  string name;
  string arm;
};

int vdctest() {
  //--- Set up the run we want to replay ---
  // This often requires a bit of coding to search directories, test
  // for non-existent files, etc.

  int run_number;
  vector<Experiment> experiments = { {"gmp",  "R"} };
  cout << "Here are the data files:" << endl;
  if( system("ls $DATA_DIR") != 0)
    return 1;

  // Get the run number
  cout << "Run number? ";
  cin >> run_number;
  if( run_number < 0 )
    return 0;

  // Check if the file with the requested run number exists
  // Assume Hall A-style raw data files ending with .dat.0
  string data_dir = gSystem->Getenv("DATA_DIR");
  if( data_dir.empty() )
    data_dir = ".";
  string run_file;
  Experiment experiment;
  bool found = false;
  for( const Experiment& ex : experiments ) {
    ostringstream ostr;
    ostr << data_dir << "/" << ex.name << "_" << run_number << ".dat.0";
    run_file = ostr.str();
    if( !gSystem->AccessPathName(run_file.c_str()) ) {
      found = true;
      experiment = ex;
      break;
    }
  }
  if( found ) {
    Info( "vdctest.C", "Replaying %s", run_file.c_str() );
  } else {
    Error( "vdctest.C", "Can't find run number %d", run_number );
    return 1;
  }

  THaRun* run = new THaRun( run_file.c_str(),
			    Form("%s run %d", experiment.name.c_str(),
				 run_number) );

  //--- Define the experimental configuration, i.e. spectrometers, detectors ---

  const char* const arm = experiment.arm.c_str();
  string oa = "O"+experiment.arm;
  const char* const oldarm = oa.c_str();
  gHaTextvars->Set("arm",arm);
  gHaTextvars->Set("oldarm",oldarm);
  THaHRS* hrs = new THaHRS(arm, Form("%sHRS",arm));
  THaVDC* vdc = new THaVDC("vdc", Form("%sHRS Vertical Drift Chambers",arm));
  hrs->AddDetector(vdc);
  //  vdc->SetDebug(3);
  //  vdc->SetErrorCutoff(5);
  hrs->AddDetector( new THaScintillator("s2", Form("%sHRS S2m scintillator",arm)));
  hrs->AddDetector( new THaCherenkov("cer", Form("%sHRS gas Cherenkov",arm)));
  gHaApps->Add(hrs);

  THaHRS* oldhrs = new THaHRS(oldarm, Form("Old %sHRS",arm));
  OldVDC* oldvdc = new OldVDC("vdc", Form("Old %sHRS Vertical Drift Chambers",arm));
  oldhrs->AddDetector(oldvdc);
  gHaApps->Add(oldhrs);

  // Ideal beam (perfect normal incidence and centering)
  THaIdealBeam* ib = new THaIdealBeam("IB", "Ideal beam");
  gHaApps->Add(ib);

  // Define how many events we want to analyze. This is a property of the run
  // because the ranges of good events could be different for different runs
  int nev = -1;
  cout << "Number of events to replay (-1=all)? ";
  cin >> nev;
  if( nev > 0 )
    run->SetLastEvent(nev);

  //--- Set up any physics calculations we want to do ---

  // Vertex position calculated from HRS golden track and ideal beam
  // (will have poor resolution if raster is on)
  gHaPhysics->Add( new THaReactionPoint( Form("%s.vx",arm),
					 Form("Vertex %s",arm),
					 arm, "IB" ));
  gHaPhysics->Add( new THaReactionPoint( Form("%s.vx",oldarm),
					 Form("Old Vertex %s",arm),
					 oldarm, "IB" ));

  //--- Define what we want the analyzer to do ---
  // The only mandatory items are the output definition and output file names

  THaAnalyzer* analyzer = new THaAnalyzer;

  string out_dir = gSystem->Getenv("OUT_DIR");
  if( out_dir.empty() )
    out_dir = ".";
  ostringstream ostr;
  ostr << out_dir << "/" << experiment.name << "_" << run_number << "_vdctest.root";
  string out_file = ostr.str();
  ostringstream ostr2;
  ostr2 << out_dir << "/" << experiment.name << "_" << run_number << "_vdctest.cuts";
  string summary_file = ostr2.str();

  analyzer->SetOutFile( out_file.c_str() );

  analyzer->SetCutFile( "vdctest.cdef" );
  analyzer->SetOdefFile( "vdctest.odef" );
  analyzer->SetSummaryFile( summary_file.c_str() );
  analyzer->SetVerbosity(2);  // write cut summary to stdout
  analyzer->EnableBenchmarks();
  analyzer->EnableSlowControl(false);
  analyzer->EnableHelicity(false);

  // Initialize (not explicitly needed, if not done, Process() will do)
  analyzer->Init(run);

  // Print what we have defined
  gHaApps->Print("DETS");
  gHaPhysics->Print();
  gHaVars->Print();

  //--- Analyze the run ---
  // Here, one could replay more than one run with
  // a succession of Process calls. The results would all go into the
  // same ROOT output file

  analyzer->Process(run);

  // Clean up

  analyzer->Close();
  delete analyzer;
  gHaCuts->Clear();
  gHaVars->Delete();
  gHaPhysics->Delete();
  gHaApps->Delete();

  // Open the ROOT file so that a user doing interactive analysis can
  // immediately look at the results. Of course, don't do this in batch jobs!
  // To close the file later, simply type "delete rootfile" or just exit

  TFile* rootfile = new TFile( out_file.c_str(), "READ" );

  return 0;
}
