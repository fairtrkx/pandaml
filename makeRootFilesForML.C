// ROOT includes
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RDF/Utils.hxx>

// C++ includes
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

using namespace ROOT;

/// @brief Display a progress bar in the terminal
/// @param current Current iteration
/// @param total Total number of iterations
void displayProgressBar(int current, int total) 
{
    // Width of the progress bar
    const int barWidth = 50;
    float progress = static_cast<float>(current) / total;

    // Calculate the number of `#` characters to display
    int pos = barWidth * progress;

    // Output the progress bar
    std::cout << " ";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) {
            std::cout << "\u2588";
        } else {
            std::cout << "\u2591";
        }
    }
    std::cout << " " << static_cast<int>(progress * 100) << "%" << " (" << current << "/" << total << ")\r"; // \r returns to the beginning of the line
    std::cout.flush(); // Flush the output buffer
}


/// @brief Returns the content of a CSV file as a 2d string vector
/// @param geoCsvFileName Name of the CSV file to read
/// @return 2d string vector containing the content of the CSV file
std::vector<std::vector<std::string>> readGeomFromCsv(const char* geoCsvFileName)
{
    // Open the CSV file
    std::ifstream geoCsvFile(geoCsvFileName);

    // Check if the file was opened successfully
    if (!geoCsvFile.is_open()) 
    {
        throw std::runtime_error(Form("Could not open file: %s", geoCsvFileName));
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

/// @brief Creates a ROOT file with the data used for machine learning using PandaRoot ROOT output files.
/// @details This macro reads the simulations and digitization files ROOT files created by the PandaRoot simulations 
///          chain macros and writes the data relevant for machine learning to a new ROOT file. 
///          Additional information is extracted using the STT geometry data. If no simulation ROOT file can be found
///          with the specified prefix, only the digitization data is written to the output file.
/// @param prefix Path to and name of the simulation and digitization files. E.g. "path/to/root/myData". It does not accept ~ as home directory!
/// @param outputDir Path to the output directory where a ROOT file called "mlData.root" will be created. It does not accept ~ as home directory!
/// @return Returns 1 if the ROOT file was created successfully, otherwise 0.
int makeRootFilesForML(std::string prefix, std::string outputDir)
{
    // Check if the prefix or output directory string contains has a "~" as its first character
    if (prefix[0] == '~' || outputDir[0] == '~')
    {
        std::cerr << "ERROR: The prefix and output directory string cannot start with a '~' character!" << std::endl;
        return 0;
    }

    // Start timer to measure the elapsed time
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Get the STT geometry data..." << std::endl;

    // 2d string vector containing the STT geometry data
    auto sttGeoData = readGeomFromCsv("/home/nikin105/mlProject/data/detectorGeometries/tubePos.csv");

    // Get the simulation and digitization files
    std::cout << std::endl;
    std::cout << "Checking if the files exist with the specified prefix..." << std::endl;
    
    // Check if the digitization file exists and get the pndsim tree
    std::string digiFilePath = prefix + "_digi.root";
    TFile *digiFile = new TFile(digiFilePath.c_str(),"READ");
    bool digiFileExists = digiFile->IsOpen();
    TTree *inputTree;
    if (digiFileExists)
    {
        std::cout << Form("Digitization file %s found!",digiFilePath.c_str()) << std::endl;
        inputTree = (TTree*)digiFile->Get("pndsim");
    }
    else
    {
        std::cout << Form("ERROR: Digitization file %s not found!",digiFilePath.c_str()) << std::endl;
        std::cout << "ERROR: A digitization file must always be provided!" << std::endl;
        return 0;
    }

    // Check if the simulation file exists and get the pndsim tree and merge it with the digi tree if it exists
    std::string simFilePath  = prefix + "_sim.root";
    TFile *simFile = new TFile(simFilePath.c_str(),"READ");
    bool simFileExists = simFile->IsOpen();
    TTree *simTree;
    if (simFileExists)
    {
        std::cout << Form("Simulation file %s found!",simFilePath.c_str()) << std::endl;
        simTree = (TTree*)simFile->Get("pndsim");
        inputTree->AddFriend(simTree);
    }
    else
    {
        std::cout << Form("Simulation file %s not found!",simFilePath.c_str()) << std::endl;
        std::cout << "Only digitization data will be written to the output file." << std::endl;
    }

    // Create a ROOT data frame for with the input tree
    RDataFrame df(*inputTree);

    std::cout << std::endl;
    std::cout << "Write the data to ROOT file..." << std::endl;
    std::cout << "Output File: " << outputDir << "/mlData.root" << std::endl;

    // Create the output ROOT file and check if it was created successfully
    TFile* outputFile = new TFile(Form("%s/mlData.root",outputDir.c_str()), "RECREATE");
    if (!outputFile || outputFile->IsZombie()) 
    {
        std::cerr << "Failed to open destination file!" << std::endl;
        return 0;
    }

    // Define ROOT double and integer vectors which will be later filled with the data
    RVecD v_x, v_y, v_z, v_dep_charge, v_isochrone, v_energy_loss, 
          v_tx, v_ty, v_tz, v_tT, v_tpx, v_tpy, v_tpz,
          v_vx, v_vy, v_vz, v_px, v_py, v_pz, v_start_time;
    
    RVecI v_hit_id, v_volume_id, v_module_id, v_layer_id, v_sector_id,
          v_skewed, v_particle_id, v_nhits, v_pdgcode, v_primary, v_particle_num;

    // Create the trees for the output ROOT file
    TTree *truth, *particles;

    if(simFileExists)
    {
        truth     = new TTree("truth"    , "truth");
        particles = new TTree("particles", "particles");
    }
    
    TTree *cells = new TTree("cells"    , "cells");
    TTree *hits  = new TTree("hits"     , "hits");
    
    // Create the branches for the "hits" tree
    TBranch* b_hits_hit_id    = hits->Branch("hit_id"   , &v_hit_id);
    TBranch* b_hits_x         = hits->Branch("x"        , &v_x);
    TBranch* b_hits_y         = hits->Branch("y"        , &v_y);
    TBranch* b_hits_z         = hits->Branch("z"        , &v_z);
    TBranch* b_hits_volume_id = hits->Branch("volume_id", &v_volume_id);
    TBranch* b_hits_layer_id  = hits->Branch("layer_id" , &v_layer_id);
    TBranch* b_hits_module_id = hits->Branch("module_id", &v_module_id);

    // Create the branches for the "cells" tree
    TBranch* b_cells_hit_id      = cells->Branch("hit_id"     , &v_hit_id);
    TBranch* b_cells_dep_charge  = cells->Branch("dep_charge" , &v_dep_charge);
    TBranch* b_cells_energy_loss = cells->Branch("energy_loss", &v_energy_loss);
    TBranch* b_cells_volume_id   = cells->Branch("volume_id"  , &v_volume_id);
    TBranch* b_cells_layer_id    = cells->Branch("layer_id"   , &v_layer_id);
    TBranch* b_cells_module_id   = cells->Branch("module_id"  , &v_module_id);
    TBranch* b_cells_sector_id   = cells->Branch("sector_id"  , &v_sector_id);
    TBranch* b_cells_isochrone   = cells->Branch("isochrone"  , &v_isochrone);
    TBranch* b_cells_skewed      = cells->Branch("skewed"     , &v_skewed);

    // Create the branches for the "truth" tree
    TBranch *b_truth_hit_id, *b_truth_x, *b_truth_y, *b_truth_z, *b_truth_t, *b_truth_px, *b_truth_py, *b_truth_pz, *b_truth_particle_id;
    
    if(simFileExists)
    {
        b_truth_hit_id      = truth->Branch("hit_id"     , &v_hit_id);
        b_truth_x           = truth->Branch("tx"         , &v_tx);
        b_truth_y           = truth->Branch("ty"         , &v_ty);
        b_truth_z           = truth->Branch("tz"         , &v_tz);
        b_truth_t           = truth->Branch("tT"         , &v_tT);
        b_truth_px          = truth->Branch("tpx"        , &v_tpx);
        b_truth_py          = truth->Branch("tpy"        , &v_tpy);
        b_truth_pz          = truth->Branch("tpz"        , &v_tpz);
        b_truth_particle_id = truth->Branch("particle_id", &v_particle_id);
    }
    
    // Create the branches for the "particles" tree
    TBranch *b_particles_particle_id, *b_vx, *b_vy, *b_vz, *b_px, *b_py, *b_pz, *b_nhits, *b_pdgcode, *b_start_time, *b_primary;

    if(simFileExists)
    {
        b_particles_particle_id = particles->Branch("particle_id", &v_particle_num);
        b_vx                    = particles->Branch("vx"         , &v_vx);
        b_vy                    = particles->Branch("vy"         , &v_vy);
        b_vz                    = particles->Branch("vz"         , &v_vz);
        b_px                    = particles->Branch("px"         , &v_px);
        b_py                    = particles->Branch("py"         , &v_py);
        b_pz                    = particles->Branch("pz"         , &v_pz);
        b_nhits                 = particles->Branch("nhits"      , &v_nhits);
        b_pdgcode               = particles->Branch("pdgcode"    , &v_pdgcode);
        b_start_time            = particles->Branch("start_time" , &v_start_time);
        b_primary               = particles->Branch("primary"    , &v_primary);
    }

    // get the total number of events in the input tree
    int nEvents = inputTree->GetEntries();

    // Event counter for the progress bar
    int evtNum = 1;

    // Lambda function to fill the "hits" and "cells" ROOT trees
    auto fillDigiTrees = 
    [
        hits, cells, sttGeoData, nEvents, &evtNum,
        &v_hit_id, &v_x, &v_y, &v_z, &v_volume_id, &v_layer_id, &v_module_id,
        &v_dep_charge, &v_energy_loss, &v_sector_id, &v_isochrone, &v_skewed
    ]
    (
        RVecI iv_hit_id, RVecD iv_x, RVecD iv_y, RVecD iv_z, RVecI iv_volume_id, RVecI iv_module_id,
        RVecD iv_dep_charge, RVecD iv_isochrone
    ) -> void
    {
        // Hits
        v_hit_id    = iv_hit_id;
        v_x         = iv_x;
        v_y         = iv_y;
        v_z         = iv_z;
        v_volume_id = iv_volume_id;
        v_module_id = iv_module_id;

        v_layer_id.clear();
        for(RVecI::iterator moduleID = v_module_id.begin(); moduleID != v_module_id.end(); ++moduleID)
            v_layer_id.push_back(stoi(sttGeoData.at(*moduleID-1).at(1)));

        // Cells
        v_dep_charge  = iv_dep_charge;
        v_energy_loss = iv_dep_charge / 1e6;

        v_sector_id.clear();
        for(RVecI::iterator moduleID = v_module_id.begin(); moduleID != v_module_id.end(); ++moduleID)
            v_sector_id.push_back(stoi(sttGeoData.at(*moduleID-1).at(2)));

        v_isochrone = iv_isochrone;

        v_skewed.clear();
        for(RVecI::iterator moduleID = v_module_id.begin(); moduleID != v_module_id.end(); ++moduleID)
            v_skewed.push_back(stoi(sttGeoData.at(*moduleID-1).at(10)));

        hits ->Fill();
        cells->Fill();

        displayProgressBar(evtNum, nEvents);
        evtNum++;
    };

    // Lambda function to fill the "truth", "particles", "hits", and "cells" ROOT trees
    auto fillDigiAndSimTrees = 
    [
        truth, particles, nEvents, &evtNum,
        &v_hit_id, &v_tx, &v_ty, &v_tz, &v_tT, &v_tpx, &v_tpy, &v_tpz, &v_particle_id, &v_particle_num,
        &v_vx, &v_vy, &v_vz, &v_px, &v_py, &v_pz, &v_nhits, &v_pdgcode, &v_start_time, &v_primary,
        hits, cells, sttGeoData,
        &v_x, &v_y, &v_z, &v_volume_id, &v_layer_id, &v_module_id,
        &v_dep_charge, &v_energy_loss, &v_sector_id, &v_isochrone, &v_skewed
    ]
    (
        RVecD iv_tx, RVecD iv_ty, RVecD iv_tz, RVecD iv_tT, RVecD iv_tpx, RVecD iv_tpy, RVecD iv_tpz, RVecI iv_particle_id,
        RVecD iv_vx, RVecD iv_vy, RVecD iv_vz, RVecD iv_px, RVecD iv_py, RVecD iv_pz, RVecI iv_nhits, RVecI iv_pdgcode, RVecD iv_start_time, RVecI iv_generator_flag,
        RVecI iv_hit_id, RVecD iv_x, RVecD iv_y, RVecD iv_z, RVecI iv_volume_id, RVecI iv_module_id,
        RVecD iv_dep_charge, RVecD iv_isochrone
    )
    {
        // Truth
        v_hit_id.clear();
        for (int i = 0; i < iv_tx.size(); i++)
            v_hit_id.push_back(i);

        v_tx          = iv_tx;
        v_ty          = iv_ty;
        v_tz          = iv_tz;
        v_tT          = iv_tT;
        v_tpx         = iv_tpx;
        v_tpy         = iv_tpy;
        v_tpz         = iv_tpz;
        v_particle_id = iv_particle_id;

        // Particles
        v_particle_num.clear();
        for (int i = 0; i < iv_vx.size(); i++)
            v_particle_num.push_back(i);

        v_vx         = iv_vx;
        v_vy         = iv_vy;
        v_vz         = iv_vz;
        v_px         = iv_px;
        v_py         = iv_py;
        v_pz         = iv_pz;
        v_nhits      = iv_nhits;
        v_pdgcode    = iv_pdgcode;
        v_start_time = iv_start_time;

        v_primary.clear();
        for(RVecI::iterator generator_flag = iv_generator_flag.begin(); generator_flag != iv_generator_flag.end(); ++generator_flag)
        {
            if(*generator_flag == 0)
                v_primary.push_back(0);
            else
                v_primary.push_back(1);
        }

        truth    ->Fill();
        particles->Fill();

        // Hits
        v_hit_id    = iv_hit_id;
        v_x         = iv_x;
        v_y         = iv_y;
        v_z         = iv_z;
        v_volume_id = iv_volume_id;
        v_module_id = iv_module_id;

        v_layer_id.clear();
        for(RVecI::iterator moduleID = v_module_id.begin(); moduleID != v_module_id.end(); ++moduleID)
            v_layer_id.push_back(stoi(sttGeoData.at(*moduleID-1).at(1)));

        // Cells
        v_dep_charge  = iv_dep_charge;
        v_energy_loss = iv_dep_charge / 1e6;

        v_sector_id.clear();
        for(RVecI::iterator moduleID = v_module_id.begin(); moduleID != v_module_id.end(); ++moduleID)
            v_sector_id.push_back(stoi(sttGeoData.at(*moduleID-1).at(2)));

        v_isochrone = iv_isochrone;

        v_skewed.clear();
        for(RVecI::iterator moduleID = v_module_id.begin(); moduleID != v_module_id.end(); ++moduleID)
            v_skewed.push_back(stoi(sttGeoData.at(*moduleID-1).at(10)));

        hits ->Fill();
        cells->Fill();

        displayProgressBar(evtNum, nEvents);
        evtNum++;
    };
    
    // vector with the names of the needed parameters / columns in the pndsim tree of the "_sim" ROOT file
    std::vector<std::string> simParamNames = 
    {
        "STTPoint.fX", "STTPoint.fY", "STTPoint.fZ", "STTPoint.fTime", "STTPoint.fPx", "STTPoint.fPy", "STTPoint.fPz", "STTPoint.fTrackID",
        "MCTrack.fStartX", "MCTrack.fStartY", "MCTrack.fStartZ", "MCTrack.fPx", "MCTrack.fPy", "MCTrack.fPz", "MCTrack.fPoints", "MCTrack.fPdgCode", "MCTrack.fStartT", "MCTrack.fGeneratorFlags"
    };

    // vector with the names of the needed parameters / columns in the pndsim tree of the "_digi" ROOT file
    std::vector<std::string> digiParamNames = 
    {
        "STTHit.fRefIndex", "STTHit.fX", "STTHit.fY", "STTHit.fZ", "STTHit.fDetectorID", "STTHit.fTubeID",
        "STTHit.fDepCharge", "STTHit.fIsochrone",
    };

    if(simFileExists)
    {
        std::cout << "Filling the \"cells\", \"hits\", \"truth\", and \"particle\" trees..." << std::endl;
        
        // Combine the simParamNames and digiParamNames vectors
        simParamNames.insert(simParamNames.end(), digiParamNames.begin(), digiParamNames.end());
        
        // Iterate over the data frame and fill the ROOT trees
        df.Foreach(fillDigiAndSimTrees , simParamNames);
        std::cout << std::endl;
        
        // Write the trees to the output ROOT file
        cells    ->Write();
        hits     ->Write();
        truth    ->Write();
        particles->Write();
    }
    else
    {
        std::cout << "Filling the \"cells\" and \"hits\" trees..." << std::endl;
        
        // Iterate over the data frame and fill the ROOT trees
        df.Foreach(fillDigiTrees, digiParamNames);
        std::cout << std::endl;

        // Write the trees to the output ROOT file
        cells->Write();
        hits ->Write();
    }

    // Save and close the output ROOT file
    outputFile->Write();
    outputFile->Close();

    // Stop the timer and print the elapsed time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds." << std::endl;

    return 1;
}