using namespace std;

// c++ includes
#include <iostream>

// ROOT includes
#include <TROOT.h>
#include <TClonesArray.h>

// FairRoot includes
#include <FairRuntimeDb.h>

// PandaRoot includes
#include <PndMasterRunAna.h>
#include <PndGeoSttPar.h>
#include <PndSttMapCreator.h>
#include <PndSttTube.h>
#include <PndSttGeometryMap.h>

void pndSttGeoToCvs()
{
	// Set the names of the input/output files
	// These are mostly dummy files but needed for the MasterRunAna class, 
	// In one of the input files the geometry parameters are stored, but I don't know where
	TString prefix 		 = "../helperFiles/geoSim";	// Prefix for the input/output files
	TString parAsciiFile = "all.par";   			// File that contains all detector parameters
	TString input        = "";          			// (Decay) file with which the simulation was performed
	TString friend1		 = "sim"; 					// Input file 1 (sim root file)
	TString output       = "digi";					// Output file (digi root file)

	// Use the MasterRunAna class to set up the run and, most importantly, import the geometry
	PndMasterRunAna *fRun= new PndMasterRunAna();
	
	// Set up the run
	LOG(INFO) << "Starting setup...";
	fRun->SetInput(input);					// Input (decay) file used for the simulation
	fRun->AddFriend(friend1);				// Input file 1 (sim root file)
	fRun->SetOutput(output);				// Output file (digi root file)
	fRun->SetParamAsciiFile(parAsciiFile);	// Input file with detector parameters
	fRun->Setup(prefix);
	LOG(INFO) << "Finished setup";

	// Get the STT geometry parameters from the runtime database
	FairRuntimeDb *rtdb = FairRun::Instance()->GetRuntimeDb();
  	PndGeoSttPar *fSttParameters = (PndGeoSttPar *)rtdb->getContainer("PndGeoSttPar");

	// Start the initialization of the run
	LOG(INFO) << "Initialising the run...";
	fRun->Init();
	LOG(info) << "Inti finished";

	LOG(info) << "Start writing the .csv file";
	// Check the geometry type (1 is .geo, 2 is .root)
	// This is mostly done to check if the geometry parameters are loaded correctly
	Int_t fGeoType = fSttParameters->GetGeometryType();
	std::cout << "Geometry type: " << fGeoType << std::endl;

	// Create the STT geometry map
	PndSttMapCreator *mapper = new PndSttMapCreator(fSttParameters);

	// Using the map, fill an array with STT tube objects
    TClonesArray *fTubeArray = mapper->FillTubeArray();
	
	// Get the number of tubes
	int numTubes = fTubeArray->GetEntries();
	
	// Create and open the output .csv file
	string outputCsvFileName = "tubePos.csv";
	printf("Output file: %s \n",outputCsvFileName.c_str());
	std::ofstream outputCsvFile (outputCsvFileName);

	// Write the header of the .csv file
	outputCsvFile << "tubeID,layerID,sectorID,x,y,z,innerRadius,outerRadius,length,angle,skewed\n";

	// Iterate over all STT tubes / tubeIDs (starting at 1 because the entry 0 is empty)
	for (int tubeID = 1; tubeID <= numTubes; tubeID++)
	{
		// Get the tube object
		PndSttTube *tube = (PndSttTube*)fTubeArray->At(tubeID);
		
		// Extract the tube parameters
		TVector3 translationVector = tube->GetPosition();
		Int_t layerID = tube->GetLayerID();
		Int_t sectorID = tube->GetSectorID(); 
		Double_t tubeInnerRadius = tube->GetRadIn();
		Double_t tubeOuterRadius = tubeInnerRadius + tube->GetRadOut();
		Double_t tubeLength = 2.*tube->GetHalfLength();
		
		// Write the tube parameters to the .csv file
		outputCsvFile	<< tubeID 				<< "," << layerID 				<< "," << sectorID 			   << "," 
						<< translationVector[0] << "," << translationVector[1]  << "," << translationVector[2] << "," 
						<< tubeInnerRadius		<< "," << tubeOuterRadius 		<< "," << tubeLength		   << ",";

		// Get the rotation angle for the skewed straw tubes
		if(tube->IsSkew())
		{
			// Get the rotation matrix
			TMatrixT<Double_t> rotationMatrix = tube->GetRotationMatrix();
			
			// Calculate the trace of the rotation matrix
			double traceRotationMatrix = rotationMatrix[0][0] + rotationMatrix[1][1] + rotationMatrix[2][2];
			
			// Calculate the absolute rotation angle and convert it to degrees
			double absRotationAngle = acos((traceRotationMatrix-1)/2);
			double absRotationAngleDegree = absRotationAngle/(2*M_PI)*360.0;
			
			// Calculate the rotation vector
			double rotationVector[3] = {rotationMatrix[2][1]-rotationMatrix[1][2],
										rotationMatrix[0][2]-rotationMatrix[2][0],
										rotationMatrix[1][0]-rotationMatrix[0][1]};

			// Check if the rotation is in the same direction as the translation vector to decide the sign of the angle
			if(	 std::signbit(rotationVector[0]) == std::signbit(translationVector[0]) && 
				(std::signbit(rotationVector[1]) == std::signbit(translationVector[1]) || 
				 rotationVector[1] == 0))
			{
				// Write the angle to the .csv file
				outputCsvFile << absRotationAngleDegree << "," <<  1 << "\n";
			}
			else
			{
				// Write the angle to the .csv file
				outputCsvFile << -1.*absRotationAngleDegree << "," << -1 << "\n";
			}
		}
		else
		{
			// Write the angle to the .csv file
			outputCsvFile << "0,0\n";
		}
	}

	outputCsvFile.close();

	fRun->Finish();
}
