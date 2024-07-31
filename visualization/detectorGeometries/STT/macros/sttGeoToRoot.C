using namespace std;

// ROOT includes
#include <TROOT.h>

// FairRoot includes
#include <FairRunSim.h>
#include <FairGenericVMCConfig.h>
#include <FairRootFileSink.h>

// PandaRoot includes
#include <PndCave.h>
#include <PndStt.h>

// Functions
void sttGeoToRoot(string geoInputFile);
string removePathAndExtension(const std::string& filepath);

/**
 * @brief Macro to convert a STT .geo file to a .root file
 * This macro converts a HADES style STT geometry .geo file into ROOT TGeo .root file
 * 
 * @param geoInputFile .geo file to be converted
 */
void sttGeoToRoot(string geoInputFile)
{
    string geoInputFileName = removePathAndExtension(geoInputFile);
    string dummyOutFile = "../helperFiles/dummy.root"; // Dummy .root files as FairRunSim requires an output file
	
    // Create a simulation run to configure the dectector simulation
    FairRunSim *run = new FairRunSim();
	run->SetName("TGeant3");   // Use GEANT4 as the transport engine
    // run->SetIsMT(false);       // Use single thread
	
    // Set the virtual MC configuration
    FairGenericVMCConfig *config = new FairGenericVMCConfig();
	run->SetSimulationConfig(std::move(config));
	
    // Set the output file
    FairRootFileSink* sink = new FairRootFileSink(dummyOutFile);
	run->SetSink(sink);

    // Get the runtime database
	FairRuntimeDb* rtdb = run->GetRuntimeDb();

    // Set the materials used for the geometry
	run->SetMaterials("media_pnd.geo"); // Media file in the $VMCWORKDIR
	
    // Set the cave geometry
	FairModule* pandaCave = new PndCave("CAVE");
	pandaCave->SetGeometryFileName("pndcave.geo"); // Cave geometry file in the $VMCWORKDIR
	run->AddModule(pandaCave);

    // Create the STT geometry from the .geo file
	FairDetector* stt = new PndStt("STT", kTRUE);
	stt->SetGeometryFileName(geoInputFile);
	run->AddModule(stt);
	
    // Initiate the simulation run and create the STT .root geometry file with the same name as the input .geo file
    run->Init();
    TString rootOutFile = geoInputFileName + ".root";
    printf("Creating geometry file: %s\n", rootOutFile.Data());
	run->CreateGeometryFile(rootOutFile);
}

/**
 * @brief Removes path and file extensions
 * Removes every caracter before the last "/" and after the last '.' in a given string
 * 
 * @param filename Input string
 * @return string Output the input string without the path and the file extension
 */
string removePathAndExtension(const string& filepath) 
{
    // Find the last directory separator
    size_t lastSlashPos = filepath.find_last_of("/");
    string baseFilename = (lastSlashPos == string::npos) ? filepath : filepath.substr(lastSlashPos + 1);

    // Find the last dot in the base filename
    size_t lastDotPos = baseFilename.find_last_of('.');
    if (lastDotPos != string::npos) {
        baseFilename = baseFilename.substr(0, lastDotPos);
    }

    return baseFilename;
}