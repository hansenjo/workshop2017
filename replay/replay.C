// Simple example replay script
//
// Ole Hansen, 22 June 2017

#include "TString.h"
#include <cstring>
#include "THaRun.h"
#include "THaAnalyzer.h"
#include "THaGlobals.h"
#include <iostream>
#include "TSystem.h"
#include "TError.h"
#include "TFile.h"
#include "THaTextvars.h"
#include "THaHRS.h"
#include "THaIdealBeam.h"
#include "THaVDC.h"
#include "THaGoldenTrack.h"
#include "THaPrimaryKine.h"
#include "THaReactionPoint.h"
#include "THaVarList.h"
#include "THaCutList.h"

using namespace std;

int replay() {
  //--- Set up the run we want to replay ---

  // This often requires a bit of coding to search directories, test
  // for non-existent files, etc.

  int run_number = 23062;
  const char* experiments[] = { "gmp", "g2p", 0 };
  const char* arms[]        = { "R", "L", 0 };
  cout << "Here are the data files:" << endl;
  if( system("ls $DATA_DIR") != 0)
    return 1;

  // Get the run number
  cout << "Run number? ";
  cin >> run_number;

  // Check if the input file exists
  TString data_dir = gSystem->Getenv("DATA_DIR");
  if( data_dir.IsNull() ) 
    data_dir = ".";
  const char** experiment = experiments;
  TString run_file;
  Bool_t found = false;
  while( *experiment ) {
    run_file = data_dir + "/" + *experiment + Form("_%d.dat.0",run_number);
    if( !gSystem->AccessPathName(run_file) ) {
      found = true;
      break;
    }
    ++experiment;
  }
  if( found ) {
    Info( "replay.C", "Replaying %s", run_file.Data() );
  } else {
    Error( "replay.C", "Can't find run number %d", run_number );
    return 1;
  }

  THaRun* run = new THaRun( run_file, Form("%s run %d optics data",
					   *experiment, run_number ) );

  //--- Define the experimental configuration, i.e. spectrometers, detectors ---

  const char* arm = arms[experiment-experiments];
  gHaTextvars->Set("arm",arm);
  THaHRS* hrs = new THaHRS(arm, Form("%sHRS",arm));
  THaVDC* vdc = dynamic_cast<THaVDC*>(hrs->GetDetector("vdc"));
  if( !vdc ) {
    vdc = new THaVDC("vdc", "Vertical Drift Chambers");
    hrs->AddDetector(vdc);
  }
  //  vdc->SetDebug(3);
  //  vdc->SetErrorCutoff(5);
  gHaApps->Add(hrs);

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

  // Extract the reconstructed target quantities of the golden track
  // Not really a physics calculation, but a convenience function.
  // It effectively converts the L.tr.* variables, which are arrays, 
  // to scalers L.gold.*

  gHaPhysics->Add( new THaGoldenTrack( Form("%s.gold",arm),
				       Form("%sHRS golden track",arm)
				       , arm ));

  // Single-arm electron kinematics for the one spectrometer we have set up.
  // We assume a carbon-12 target (12 AMU)
  gHaPhysics->Add( new THaPrimaryKine( Form("%s.ekine",arm),
				       Form("%sHRS electron kinematics",arm),
				       arm, 0.511e-3, 12*0.9315 ));

  // Vertex position calculated from RHRS golden track and ideal beam
  // (will poor resolution if raster is on)
  gHaPhysics->Add( new THaReactionPoint( Form("%s.vx",arm),
					 Form("Vertex %s",arm),
					 arm, "IB" ));

  //--- Define what we want the analyzer to do ---
  // The only mandatory items are the output definition and output file names

  gHaApps->Print();
  gHaPhysics->Print();

  THaAnalyzer* analyzer = new THaAnalyzer;

  TString out_dir = gSystem->Getenv("OUT_DIR");
  if( out_dir.IsNull() )
    out_dir = ".";
  TString out_file = out_dir + "/" + *experiment + Form("_%d.root", run_number);

  analyzer->SetOutFile( out_file );

  analyzer->SetCutFile( "replay.cdef" );
  analyzer->SetOdefFile( "replay.odef" );

  analyzer->SetVerbosity(2);  // write cut summary to stdout
  analyzer->EnableBenchmarks();

  //--- Analyze the run ---
  // Here, one could replay more than one run with
  // a succession of Process calls. The results would all go into the
  // same ROOT output file

  analyzer->Process(run);

  // Clean up

  analyzer->Close();
  delete analyzer;
  gHaCuts->Clear();
  gHaVars->Clear();
  gHaPhysics->Delete();
  gHaApps->Delete();

  // Open the ROOT file so that a user doing interactive analysis can 
  // immediately look at the results. Of course, don't do this in batch jobs!
  // To close the file later, simply type "delete rootfile" or just exit

  TFile* rootfile = new TFile( out_file, "READ" );

  return 0;
}
