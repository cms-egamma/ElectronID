#include "TROOT.h"
#include "TSystem.h"
#include "TString.h"
#include "OptimizationConstants.hh"
#include "VariableLimits.hh"
#include "optimize.hh"

void fourPointOptimization(bool useBarrel){

  // Define source for the initial cut range
  TString dateTag = "2019-08-23";
  TString startingCutMaxFileName        = "cuts_barrel_eff_0999_" + dateTag + ".root";
  if(!useBarrel) startingCutMaxFileName = "cuts_endcap_eff_0999_" + dateTag + ".root";

  TString namePrefix = useBarrel ? "cuts_barrel_" : "cuts_endcap_";
  TString namePass[Opt::nWP] = {"pass1_","pass2_","pass3_","pass4_"};
  TString nameTime = dateTag;

  for( int ipass = 0; ipass < Opt::nWP; ipass++){

    // This string is the file name that contains the ROOT file
    // with the VarCut object that defines the range of cut variation.
    // Note: for each subsequence pass, the previous working point
    // is used. For the first pass, the 99.9% efficient cut range is used.
    TString cutMaxFileName = startingCutMaxFileName;
    if(ipass > 0) cutMaxFileName = namePrefix + namePass[ipass-1] + nameTime + TString("_") + Opt::wpNames[ipass-1] + TString(".root");
    
    // The string below is used to construct the file names
    // to save the cut objects
    TString cutOutputBase = namePrefix;
    cutOutputBase += namePass[ipass];
    cutOutputBase += nameTime;

    // This string will be used to construct the dir for the output
    // of TMVA: the dir for weights and the filename for diagnostics
    TString trainingDataOutputBase = "training_results_";
    if(useBarrel) trainingDataOutputBase += "barrel_";
    else          trainingDataOutputBase += "endcap_";

    trainingDataOutputBase += namePass[ipass];
    trainingDataOutputBase += nameTime;

    // Use the following user-defined limits
    // (if in doubt, use the no-restrictions one defined in VariableLimits.hh)
    VarLims::VariableLimits **userDefinedCutLimits = VarLims::limitsNoRestrictions;
    if( ipass > 0 )  userDefinedCutLimits = VarLims::limitsWPAnyV1;
//  if( ipass == 3 ) userDefinedCutLimits = useBarrel ? VarLims::limitsHLTSafeBarrel : VarLims::limitsHLTSafeEndcap;

    printf("\n-----------------------------------------------------------------\n");
    printf("\n");
    printf("    Run new optimization pass  \n");
    printf("\n");
    printf(" Input file that defines cut optimization limits:");
    printf(" %s\n", cutMaxFileName.Data());
    printf(" Base for the file name of output cuts          :");
    printf(" %s\n", cutOutputBase.Data());
    printf("------------------------------------------------------------------\n\n");
    
    optimize(cutMaxFileName, cutOutputBase, trainingDataOutputBase, userDefinedCutLimits, useBarrel);    
  }
 
  // Finally, create the files containing the working point
  // by copying the appropriate pass files.
  // The first working point is output of pass1, the second of pass2, etc.
  // All other working poitns of all passes are ignored
  printf("\n");
  printf("====================================================\n");
  printf("Final definition of working points\n");
  printf("====================================================\n");
  printf("Copy files with working points info into the final locations\n");
  // The printf/etc lines below are to debug an rare odd problem
  printf("Current path:\n");
  gSystem->Exec("pwd"); 
  printf("Content of this dir:\n");
  gSystem->Exec("ls -rtl"); 
  printf("Content of cut_repository subdir\n");
  gSystem->Exec("ls -rtl cut_repository/"); 

  for(int i=0; i<Opt::nWP; i++){

    TString wpPassFileName  = Opt::cutRepositoryDir + TString("/") + namePrefix + namePass[i] + nameTime + TString("_") + Opt::wpNames[i] + TString(".root");
    TString wpFinalFileName = Opt::cutRepositoryDir + TString("/") + namePrefix + nameTime + TString("_") + Opt::wpNames[i] + TString(".root");

    TString copyCommand = TString::Format("cp %s %s", wpPassFileName.Data(), wpFinalFileName.Data());

    int result = gSystem->Exec(copyCommand); 
    int tries =0;
    while( result != 0 && tries <10) {
      printf("Copy command failed!!! The command: %s\n", copyCommand.Data());
      result = gSystem->Exec(copyCommand); 
      tries += 1;
    }

    printf("\nFinal definition for working point %s\n", Opt::wpNames[i].Data());
    printf(" file name:   %s\n", wpFinalFileName.Data());
    TFile *fwp = new TFile(wpFinalFileName);
    VarCut *thisWP = (VarCut*)fwp->Get("cuts");
    if(thisWP != 0) thisWP->printCuts();
    else            printf("???? not found????\n");
    fwp->Close();
  }
 
}

