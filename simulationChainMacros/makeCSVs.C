#include "makeCSVs.h"

/// @brief Returns the content of a CSV file as a 2d string vector
/// @param geoCsvFileName Name of the CSV file to read
/// @return 2d string vector containing the content of the CSV file
std::vector<std::vector<std::string>> readGeomFromCsv(const char* geoCsvFileName)
{
    // Open the CSV file
    std::ifstream geoCsvFile(geoCsvFileName);

    // Check if the file was opened successfully
    if (!geoCsvFile.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    
    // Declare the 2d vector to store the CSV data
    std::vector<std::vector<std::string>> geoData;
    
    // Declare a string to store the current line of the CSV file
    std::string line;

    // Skip the first line of the file as it only contains the column names
    std::getline(geoCsvFile, line);

    // Iterate over the lines of the file
    while (std::getline(geoCsvFile, line)) {
        // Declare a vector to store the field of the current line
        std::vector<std::string> row;
        
        // Initialize a string stream with the current line
        std::istringstream s(line);
        
        // Declare a string to store the current field
        std::string field;
        
        // Iterate over the fields of the current line which are separated by commas and store them in the row vector
        while (getline(s, field, ',')) {
            row.push_back(field);
        }

        // Store the row vector in the geoData vector
        geoData.push_back(row);
    }

    // Close the CSV file
    geoCsvFile.close();

    return geoData;
}

/// @brief Creates CSV files from the ROOT simulation and digitization files
/// @param prefix Path to and name of the ROOT simulation and digitization files
/// @param outputDir Path to the directory where the CSV files should be stored
/// @return 0 if the CSV files were created successfully
int makeCSVs(std::string prefix, std::string outputDir)
{
    // Start timer to measure the elapsed time
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Get the STT geometry data..." << std::endl;

    // 2d string vector containing the STT geometry data
    auto sttGeoData = readGeomFromCsv("/home/nikin105/mlProject/pandaml/visualization/detectorGeometries/STT/tubePos.csv");

    // Define an vector that contains the tube polarity (0 = straight, 1 = +3° skewed, -1 = -3° skewed)
    std::vector<int> tubePolarity;

    // Fill the vector according to the values from the geometry data
    for(int tube = 0; tube < sttGeoData.size(); tube++)
    {
        // Check if the tube is straight, skewed by +3° or skewed by -3°
        if(std::stoi(sttGeoData.at(tube).at(9)) == 0)
            tubePolarity.push_back(0);
        else
            tubePolarity.push_back(std::stoi(sttGeoData.at(tube).at(9))/abs(std::stoi(sttGeoData.at(tube).at(9)))); // +-1 = a / abs(a)
    }

    std::cout << std::endl;
    std::cout << "Read the simulation and digitization files..." << std::endl;
    std::cout << "Simulation File: " << prefix + "_sim.root" << std::endl;
    std::cout << "Digitalization File: " << prefix + "_digi.root" << std::endl;

    // Create a ROOT data frame for the simulation and digitization files
    RDataFrame dfSim ("pndsim", prefix + "_sim.root" );
    RDataFrame dfDigi("pndsim", prefix + "_digi.root");

    // Take the sim parameters from the data frame
    auto mcTrackPx            = dfSim.Take<RVecD>("MCTrack.fPx"            );
    auto mcTrackPy            = dfSim.Take<RVecD>("MCTrack.fPy"            );
    auto mcTrackPz            = dfSim.Take<RVecD>("MCTrack.fPz"            );
    auto mcTrackStartX        = dfSim.Take<RVecD>("MCTrack.fStartX"        );
    auto mcTrackStartY        = dfSim.Take<RVecD>("MCTrack.fStartY"        );
    auto mcTrackStartZ        = dfSim.Take<RVecD>("MCTrack.fStartZ"        );
    auto mcTrackStartT        = dfSim.Take<RVecD>("MCTrack.fStartT"        );
    auto mcTrackGeneratorFlag = dfSim.Take<RVecI>("MCTrack.fGeneratorFlags");
    auto mcTrackPdgCode       = dfSim.Take<RVecI>("MCTrack.fPdgCode"       );
    auto mcTrackPoints        = dfSim.Take<RVecI>("MCTrack.fPoints"        );

    auto sttPointX       = dfSim.Take<RVecD>("STTPoint.fX"      );
    auto sttPointY       = dfSim.Take<RVecD>("STTPoint.fY"      );
    auto sttPointZ       = dfSim.Take<RVecD>("STTPoint.fZ"      );
    auto sttPointT       = dfSim.Take<RVecD>("STTPoint.fTime"   );
    auto sttPointPx      = dfSim.Take<RVecD>("STTPoint.fPx"     );
    auto sttPointPy      = dfSim.Take<RVecD>("STTPoint.fPy"     );
    auto sttPointPz      = dfSim.Take<RVecD>("STTPoint.fPz"     );
    auto sttPointTrackID = dfSim.Take<RVecI>("STTPoint.fTrackID");
    
    // Take the digi parameters from the data frame
    auto sttHitRefIndex   = dfDigi.Take<RVecI>("STTHit.fRefIndex"  );
    auto sttHitDepCharge  = dfDigi.Take<RVecD>("STTHit.fDepCharge" );
    auto sttHitTubeID     = dfDigi.Take<RVecI>("STTHit.fTubeID"    );
    auto sttHitDetectorID = dfDigi.Take<RVecI>("STTHit.fDetectorID");
    auto sttHitIsochrone  = dfDigi.Take<RVecD>("STTHit.fIsochrone" );
    auto sttHitX          = dfDigi.Take<RVecD>("STTHit.fX"         );
    auto sttHitY          = dfDigi.Take<RVecD>("STTHit.fY"         );
    auto sttHitZ          = dfDigi.Take<RVecD>("STTHit.fZ"         );

    std::cout << std::endl;
    std::cout << "Write the data to CSV files..." << std::endl;
    std::cout << "Output Directory: " << outputDir << std::endl;

    // Define the length of the event code for the names of the CSV files
    const int codeLength = 10;

    // Initiate the number of events
    int nEvents = mcTrackPx->size();

    // Iterate over all events and write the data to four separate CSV file
    for(int event = 0; event < nEvents; event++)
    {
        // Prepare the header for the "cells" CSV file
        std::string cellsCsvFileContent = "hit_id,depcharge,energyloss,volume_id,layer_id,module_id,sector_id,isochrone,skewed\n";

        // Prepare the header for the "hits" CSV file
        std::string hitsCsvFileContent = "hit_id,x,y,z,volume_id,layer_id,module_id\n";

        // Prepare the header for the "particles" CSV file
        std::string particlesCsvFileContent = "particle_id,vx,vy,vz,px,py,pz,q,nhits,pdgcode,start_time,primary\n";

        // Prepare the header for the "truth" CSV file
        std::string truthCsvFileContent = "hit_id,tx,ty,tz,tT,tpx,tpy,tpz,weight,particle_id\n";

        // Number the CSV files according to the event number with a 10 digit code
        std::ostringstream ossEventCode;
        ossEventCode << std::setw(codeLength) << std::setfill('0') << event;
        std::string eventCode = ossEventCode.str();

        // Buffer the hit data for the "cells" and "hits" CSV files
        for(int hit = 0; hit < sttHitRefIndex->at(event).size(); hit++)
        {
            cellsCsvFileContent += std::to_string(sttHitRefIndex->at(event).at(hit))                  + ","   // reference ID that corresponds to the index of the MC point
                                +  std::to_string(sttHitDepCharge->at(event).at(hit))                 + ","   // deposited charge
                                +  std::to_string(sttHitDepCharge->at(event).at(hit) / 1e6)           + ","   // deposited energy
                                +  std::to_string(sttHitDetectorID->at(event).at(hit))                + ","   // volume ID
                                +  sttGeoData.at(sttHitTubeID->at(event).at(hit)-1).at(1)             + ","   // layer ID
                                +  std::to_string(sttHitTubeID->at(event).at(hit))                    + ","   // tube ID
                                +  sttGeoData.at(sttHitTubeID->at(event).at(hit)-1).at(2)             + ","   // sector ID
                                +  std::to_string(sttHitIsochrone->at(event).at(hit))                 + ","   // isochrone radius
                                +  std::to_string(tubePolarity.at(sttHitTubeID->at(event).at(hit)-1)) + "\n"; // polarity of the tube (0 = straight, 1 = +3° skewed, -1 = -3° skewed)

            hitsCsvFileContent += std::to_string(sttHitRefIndex->at(event).at(hit))      + ","   // reference ID that corresponds to the index of the MC point
                               +  std::to_string(sttHitX->at(event).at(hit))             + ","   // x position
                               +  std::to_string(sttHitY->at(event).at(hit))             + ","   // y position
                               +  std::to_string(sttHitZ->at(event).at(hit))             + ","   // z position
                               +  std::to_string(sttHitDetectorID->at(event).at(hit))    + ","   // volume ID
                               +  sttGeoData.at(sttHitTubeID->at(event).at(hit)-1).at(1) + ","   // layer ID
                               +  std::to_string(sttHitTubeID->at(event).at(hit))        + "\n"; // tube ID
        }

        // Create and fill the "cells" CSV file
        std::ofstream cellsCsvFile(outputDir + "/event" + eventCode + "-cells.csv",std::ios::binary);
        cellsCsvFile << cellsCsvFileContent;
        cellsCsvFile.close();

        // Create and fill the "hits" CSV file
        std::ofstream hitsCsvFile(outputDir + "/event" + eventCode + "-hits.csv",std::ios::binary);
        hitsCsvFile << hitsCsvFileContent;
        hitsCsvFile.close();

        // Buffer the truth data for the "truth" CSV file
        for(int point = 0; point < sttPointX->at(event).size(); point++)
        {
            truthCsvFileContent += std::to_string(point)                                  + ","   // index of the MC point
                                +  std::to_string(sttPointX->at(event).at(point))         + ","   // x position
                                +  std::to_string(sttPointY->at(event).at(point))         + ","   // y position
                                +  std::to_string(sttPointZ->at(event).at(point))         + ","   // z position
                                +  std::to_string(sttPointT->at(event).at(point))         + ","   // time
                                +  std::to_string(sttPointPx->at(event).at(point))        + ","   // x momentum
                                +  std::to_string(sttPointPy->at(event).at(point))        + ","   // y momentum
                                +  std::to_string(sttPointPz->at(event).at(point))        + ","   // z momentum
                                +  std::to_string(1)                                      + ","   // weight
                                +  std::to_string(sttPointTrackID->at(event).at(point))   + "\n"; // particle ID
        }

        // Create and fill the "truth" CSV file
        std::ofstream truthCsvFile(outputDir + "/event" + eventCode + "-truth.csv",std::ios::binary);
        truthCsvFile << truthCsvFileContent;
        truthCsvFile.close();

        // Buffer the particle data for the "particles" CSV file
        for(int particle = 0; particle < mcTrackPx->at(event).size(); particle++)
        {
            // only save the parameters of particles that leave at least one hit in the STT
            if(sttPointTrackID->at(event).end() != std::find(sttPointTrackID->at(event).begin(), sttPointTrackID->at(event).end(), particle))
            {
                particlesCsvFileContent += std::to_string(particle)                                     + ","   // particle ID
                                        +  std::to_string(mcTrackStartX->at(event).at(particle))        + ","   // vertex x position
                                        +  std::to_string(mcTrackStartY->at(event).at(particle))        + ","   // vertex y position
                                        +  std::to_string(mcTrackStartZ->at(event).at(particle))        + ","   // vertex z position
                                        +  std::to_string(mcTrackPx->at(event).at(particle))            + ","   // x momentum at the vertex
                                        +  std::to_string(mcTrackPy->at(event).at(particle))            + ","   // y momentum at the vertex
                                        +  std::to_string(mcTrackPz->at(event).at(particle))            + ","   // z momentum at the vertex
                                        +  std::to_string(0)                                            + ","   // charge (not implemented)
                                        +  std::to_string(mcTrackPoints->at(event).at(particle))        + ","   // number of hits
                                        +  std::to_string(mcTrackPdgCode->at(event).at(particle))       + ","   // PDG code
                                        +  std::to_string(mcTrackStartT->at(event).at(particle))        + ",";  // start time
                
                // Check if the particle is a primary particle
                if(mcTrackGeneratorFlag->at(event).at(particle) == 0)
                    particlesCsvFileContent += std::to_string(0) + "\n"; // secondary particle
                else
                    particlesCsvFileContent += std::to_string(1) + "\n"; // primary particle
            }
        }

        // Create and fill the "particles" CSV file
        std::ofstream particlesCsvFile(outputDir + "/event" + eventCode + "-particles.csv",std::ios::binary);
        particlesCsvFile << particlesCsvFileContent;
        particlesCsvFile.close();

        // Print the current progress every 10% of the events
        if(event % (nEvents/10) == 0)
        {
            std::cout << "Progress: " << event/(mcTrackPx->size()/100) << "%" << std::endl;
        }
    }
    std::cout << "Progress: 100%" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds." << std::endl;
    return 0;
}