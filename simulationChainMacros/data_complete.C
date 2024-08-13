using namespace std;

// ROOT includes
#include <TROOT.h>

// std c++ includes
#include <iostream>

// Fair includes
#include <FairFileSource.h>
#include <FairRootFileSink.h>
#include <FairParAsciiFileIo.h>
#include <FairParRootFileIo.h>
#include <FairRuntimeDb.h>
#include <FairRunInfo.h>
#include <FairRunAna.h>

// PandaRoot includes
#include <PndMLTracking.h>
#include <PndEmcMapper.h>

int data_complete(Int_t nEvents=10, TString prefix="", TString outputdir="", TString assistIdeal="", Int_t Job_Id=0) {
    
    // Print the input parameters
	std::cout << std::endl;
	LOG(INFO) << "========== Input Parameters ==========";
	LOG(INFO) << "nEvents: " 	 << nEvents;
	LOG(INFO) << "prefix: " 	 << prefix.Data();
    LOG(INFO) << "outputdir: "   << outputdir.Data();
    LOG(INFO) << "assistIdeal: " << assistIdeal.Data();
    LOG(INFO) << "Job_Id: "      << Job_Id;
	std::cout << std::endl;

    // Names of the ROOT Files
    TString parFile     = prefix+"_par.root";   // Name of the parameter file
    TString simFile     = prefix+"_sim.root";   // Name of the simulation file
    TString digiFile    = prefix+"_digi.root";  // Name of the digitization file
    TString skewFile    = prefix+"_skew.root";  // Name of the digitization file after applying the skewed straws correction
    TString recoFile    = prefix+"_reco.root";  // Name of the (ideal) reconstruction file
    TString outFile     = prefix+"_data.root";  // Name of the Output file (dummy file)

    // -----  Initialization   ------------------------------------------------
    FairLogger::GetLogger()->SetLogToFile(kFALSE);
    FairRunAna *fRun = new FairRunAna();

    // Add Input File to FairFileSource
    FairFileSource *fSrc = new FairFileSource(simFile);
    fRun->SetSource(fSrc);

    // Add Friend File to FairFileSource
    fSrc->AddFriend(digiFile);

    // Add Friend File to FairFileSource
    // fSrc->AddFriend(skewFile);

    // Add Friend File to FairFileSource
    fSrc->AddFriend(recoFile);

    // Add Output File to FairRootFileSink
    FairRootFileSink *fSink = new FairRootFileSink(outFile);
    fRun->SetSink(fSink);

    // FairRuntimeDb
    FairRuntimeDb *rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo *parInput1 = new FairParRootFileIo();
    parInput1->open(parFile.Data());
    rtdb->setFirstInput(parInput1);

    // FairParAsciiFileIo
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
    TString allDigiFile = gSystem->Getenv("VMCWORKDIR"); 
    allDigiFile += "/macro/params/all.par";
    parIo1->open(allDigiFile.Data(), "in");
    rtdb->setSecondInput(parIo1);

    // -----   Add tasks   ----------------------------------------------------
    Int_t start_counter = nEvents*Job_Id;
    PndMLTracking *genDB = new PndMLTracking(start_counter, outputdir, assistIdeal);
    fRun->AddTask(genDB);

    // -----   Intialise and run   --------------------------------------------
    PndEmcMapper::Init(1);
    fRun->Init();
    fRun->Run(0,nEvents);
    return 0;
}
