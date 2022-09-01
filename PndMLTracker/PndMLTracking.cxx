/*
 * PndMLTracking.cxx
 *
 *  Created on: Nov 8, 2018
 *      Author: Adeel Akram
 */

#include <FairRootManager.h>
#include <FairRunAna.h>
#include <FairRuntimeDb.h>
#include <PndSttMapCreator.h>
#include <PndSttTube.h>
#include <PndTrackCand.h>
#include <TClonesArray.h>

#include "FairTask.h"
#include "FairMCPoint.h"
#include "PndMLTracking.h"

ClassImp(PndMLTracking)

/*
* (1) Constructor Initialization by Assignment:
* Avoid this method as it is a two step process. In this method, first
* default constructor is called (step 1) and then assignment operation
* is performed to intialize the data memebers.
*
* (2) Constructor Initialization by Inline Assignment:
* In this case, an object is instantiated with default values of data 
* members. Its a one step process, assignment is done in "*.h" files.
* NOTE: Inline assignment is only available in C++11 and beyond.
*
* (3) Constructor Initialization by Initializer List:
* In this case, an object is instantiated with default values of data 
* members. Its a one step process, default values are provided using
* and initializer list to the constructor. It is considered the BEST
* way for instantiating a statefull class object.
* NOTE: Initializer list depends on order of declaration of data members.
*/

/* PndMLTracking() */
PndMLTracking::PndMLTracking()
    : fEventId(0)
    , fCsvFilesPath("./data")
    , mcTrackBranchID(-1)
    , fMCTrackArray(nullptr)
    , mvdHitsPixelBranchID(-1)
    , fMvdHitsPixelArray(nullptr)
    , mvdHitsStripBranchID(-1)
    , fMvdHitsStripArray(nullptr)
    , gemHitBranchID(-1)
    , fGemHitArray(nullptr)
    , sttPointBranchID(-1)
    , fSttPointArray(nullptr)
    , sttHitBranchID(-1)
    , fSttHitArray(nullptr)
    , barrelTrackBranchID(-1)       // formerly SttMvdGemTrack
    , fBarrelTrackArray(nullptr)    // formerly SttMvdGemTrack
    , fSttParameters(nullptr)
    , fEventHeader(nullptr)
    , fTubeArray(nullptr) {

    /* Constructor (1) */
}

/* PndMLTracking(int) */
PndMLTracking::PndMLTracking(int start_counter, TString csv_path)
    : fEventId(start_counter)
    , fCsvFilesPath(csv_path)
    , mcTrackBranchID(-1)
    , fMCTrackArray(nullptr)
    , mvdHitsPixelBranchID(-1)
    , fMvdHitsPixelArray(nullptr)
    , mvdHitsStripBranchID(-1)
    , fMvdHitsStripArray(nullptr)
    , gemHitBranchID(-1)
    , fGemHitArray(nullptr)
    , sttPointBranchID(-1)
    , fSttPointArray(nullptr)
    , sttHitBranchID(-1)
    , fSttHitArray(nullptr)
    , barrelTrackBranchID(-1)       //formerly SttMvdGemTrack
    , fBarrelTrackArray(nullptr)    // formerly SttMvdGemTrack
    , fSttParameters(nullptr)
    , fEventHeader(nullptr)
    , fTubeArray(nullptr) {

    /* Constructor (2) */
}

/* Destructor */
PndMLTracking::~PndMLTracking() {
}

/* SetParContainers() */
void PndMLTracking::SetParContainers() {

    FairRuntimeDb *rtdb = FairRunAna::Instance()->GetRuntimeDb();
    fSttParameters = (PndGeoSttPar*) rtdb->getContainer("PndGeoSttPar");

}

/* Init() */
InitStatus PndMLTracking::Init() {

    // Get instance of the FairRootManager to access tree branches
    FairRootManager *ioman = FairRootManager::Instance();
    if(!ioman) {
        std::cout << "-E- PndMLTracking::Init: FairRootManager not instantiated!" << std::endl;
        return kFATAL;
    }
    
    // Get the EventHeader
    fEventHeader = (TClonesArray*) ioman->GetObject("EventHeader.");
    if(!fEventHeader) {
        std::cout << "-E- PndMLTracking::Init: EventHeader not loaded!" << std::endl;
        return kFATAL;
    }
    
    // Access STTMapCreater
    PndSttMapCreator *mapper = new PndSttMapCreator(fSttParameters);
    fTubeArray = mapper->FillTubeArray();
    
    // Access MCTrack branch and determine its branch ID
    fMCTrackArray = (TClonesArray*) ioman->GetObject("MCTrack");
    mcTrackBranchID = ioman->GetBranchId("MCTrack");
    
    if (!fMCTrackArray) {
        LOG(error) << " " << GetName() << "::Init: No MCTrack array!";
        return kERROR;
    }
    
    // Access STTPoint (MC) branch and determine its branch ID
    fSttPointArray = (TClonesArray*) ioman->GetObject("STTPoint");
    sttPointBranchID = ioman->GetBranchId("STTPoint");
    
    // Access MVDHitsPixel branch and determine its branch ID
    fMvdHitsPixelArray = (TClonesArray*) ioman->GetObject("MvdHitsPixel");
    mvdHitsPixelBranchID = ioman->GetBranchId("MvdHitsPixel");
    
    // Access MVDHitsPixel branch and determine its branch ID
    fMvdHitsStripArray = (TClonesArray*) ioman->GetObject("MvdHitsStrip");
    mvdHitsStripBranchID = ioman->GetBranchId("MvdHitsStrip");
    
    // Access GEMHit branch and determine its branch ID
    fGemHitArray = (TClonesArray*) ioman->GetObject("GEMHit");
    gemHitBranchID = ioman->GetBranchId("GEMHit");

    // Access STTHit branch and determine its branch ID
    fSttHitArray = (TClonesArray*) ioman->GetObject("STTHit");
    sttHitBranchID = ioman->GetBranchId("STTHit");

    // Access BarrelTrack (formerly SttMvdGemTrack) branch and determine its branch ID
    fBarrelTrackArray = (TClonesArray*) ioman->GetObject("BarrelTrack");
    barrelTrackBranchID = ioman->GetBranchId("BarrelTrack");
    
    std::cout << "-I- PndMLTracking: Initialisation successful" << std::endl;
    return kSUCCESS;

}

/* Exec() */
void PndMLTracking::Exec(Option_t* /*opt*/) {
    
    // Debugging Tasks    
    // 1 - Get STTPoint connected to STTHit
    // 2 - Get STTPoint connected to MCTrack
    // 3 - Filter STTPoints/STTHits if IsGeneratorCreated()
    // 4 - Get TrackID/MCTrackID of STTPoint/STTHit
    
    // Set Event Index
    std::stringstream ss;
    ss << std::setw(10) << std::setfill('0') << fEventId;
    std::string fidx = ss.str();
    
    /* ***********************************************************************
    *                          Open CSV Files
    *  ********************************************************************* */
    
    TString csv_path = fCsvFilesPath+"/event"+fidx;
    
    fHits.open(csv_path+"-hits.csv");
    fTruths.open(csv_path+"-truth.csv");
    fParticles.open(csv_path+"-particles.csv");
    fTubes.open(csv_path+"-cells.csv");
    
    std::cout << "\nProcessing Event: " << csv_path << std::endl;
    
    /* ***********************************************************************
    *                          Add CSV Header
    *  ********************************************************************* */

        
    /* ------------------------------------------------------------------------
    *                          (1) Event Hits
    *  --------------------------------------------------------------------- */

    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    *
    * The hits file contains the following values for each hit/entry:
    *
    *  - hit_id : numerical identifier of the hit inside the event.
    *  - x, y, z : measured x, y, z position (in mm) of the hit in global coordinates.
    *  - volume_id : numerical identifier of the detector group.
    *  - layer_id : numerical identifier of the detector layer inside the group.
    *  - module_id : numerical identifier of the detector module inside the layer.
    *
    * The volume/layer/module id could in principle be deduced from x, y, z. They are 
    * given here to simplify detector-specific data handling.
    *
    * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    fHits   << "hit_id" << ","
            << "x" << "," 
            << "y" << "," 
            << "z" << ","
            << "volume_id" << ","       // e.g. STT (sub-detectors)
            << "layer_id"  << ","       // e.g. layer_id in STT
            << "module_id"              // e.g. module_id==tube_id
            << std::endl;
    
    /* ------------------------------------------------------------------------
    *                          (2) Event Truths
    *  --------------------------------------------------------------------- */
    
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    *
    * The truth file contains the mapping between hits and generating particles 
    * and the true particle state at each measured hit. Each entry maps one hit 
    * to one particle.
    *
    * - hit_id: numerical identifier of the hit as defined in the hits file.
    * - particle_id: numerical identifier of the generating particle as defined 
    *                in the particles file. A value of 0 means that the hit did 
    *                not originate from a reconstructible 
    *                particle, but e.g. from detector noise.
    *
    * - tx, ty, tz: true intersection point in global coordinates (in mm)
    *               between the particle trajectory and the sensitive surface.
    *
    * - tpx,tpy,tpz: true particle momentum (in GeV/c) in the global coordinate 
    *                system at the intersection point. The corresponding vector 
    *               is tangent to the particle trajectory at intersection point.
    *
    * - weight: per-hit weight used for the scoring metric; total sum of weights 
    *           within one event equals to one.
    * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    
    fTruths << "hit_id" << ","
            << "tx"  << ","
            << "ty"  << ","
            << "tz"  << ","
            << "tpx" << ","
            << "tpy" << ","
            << "tpz" << ","
            << "weight" << ","
            << "particle_id"
            << std::endl;
    
    /* ------------------------------------------------------------------------
    *                          (3) Event Tubes
    *  --------------------------------------------------------------------- */
    
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    *
    * - hit_id: numerical identifier of the hit as defined in the hits file.
    * - isochrone:
    * - depcharge:
    * - energyloss:
    * - volume_id, layer_id, module_id
    * - skewed, sector_id
    * 
    * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    // Future Add: GetHalfLength(), GetWireDirection(), GetRadIn(), GetRadOut(),
    //             GetNeighborings(), GetDistance(), IsParallel()

    fTubes  << "hit_id"    << ","
            << "isochrone" << "," 
            << "depcharge" << "," 
            << "energyloss"<< ","
            << "volume_id" << ","       // e.g. STT (sub-detectors)
            << "layer_id"  << ","       // e.g. layer_id in STT
            << "module_id" << ","       // e.g. tube_id -> layer_id -> STT
            << "skewed"    << ","       // if tube_id is skewed.
            << "sector_id"              // in which sector tube exists.
            << std::endl;

    /* ------------------------------------------------------------------------
    *                          (4) Event Particles
    *  --------------------------------------------------------------------- */
    
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    *
    * The particles files contains the following values for each particle/entry:
    * 
    * - particle_id: numerical identifier of the particle inside the event.
    * - particle_type: numerical identifier of the particle type (PDG CODE)
    * - vx, vy, vz: initial position or vertex (in mm) in global coordinates.
    * - px, py, pz: initial momentum (in GeV/c) along each global axis.
    * - q: particle charge (as multiple of the absolute electron charge).
    * - nhits: number of hits generated by this particle.
    * 
    * All entries contain the generated information or ground truth.
    * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    
    fParticles  << "particle_id" <<","  // identifier of a track (track_id)
                << "vx" << ","
                << "vy" << ","
                << "vz" << ","
                << "px" << ","
                << "py" << ","
                << "pz" << ","
                << "q"  << ","
                << "nhits" << ","
                << "pdgcode" << ","
                << "start_time"
                << std::endl;
    
    /* ************************************************************************
    *                       Add CSV Data according to Header
    *  ********************************************************************* */
    
    //GenerateMvdData();
    //GenerateGemData();
    GenerateSttData();
    
    //Close CSVs
    fHits.close();
    fTruths.close();
    fParticles.close();
    fTubes.close();
    
    std::cout << "-I- PndMLTracking: Exec Successful with Event: " << (fEventId) << " with Prefix: " << "event"+fidx << std::endl;
    fEventId++;
    
}//end-Exec()


/* GenerateMvdData() */
void PndMLTracking::GenerateMvdData() { /* Under Construction */ }

/* GenerateGemData() */
void PndMLTracking::GenerateGemData() { /* Under Construction */ }


/* GenerateSttData() */
void PndMLTracking::GenerateSttData() {

    // ------------------------------------------------------------------------
    //               Data from STT (fSttHitArray) to CSVs
    // ------------------------------------------------------------------------
    
    
    
    int hit_counter = 0;
    
    for (int hit_idx=0; hit_idx < fSttHitArray->GetEntriesFast(); hit_idx++) {

        // Get FairRootManager Instance
        FairRootManager *ioman = FairRootManager::Instance();

        // Get sttHitsLinks from STTHit
        FairMultiLinkedData_Interface *sttHitsLinks = (FairMultiLinkedData_Interface*)fSttHitArray->At(hit_idx);

        // Get sttPointLinks & Data (TCloneArray) from sttHitsLinks
        FairMultiLinkedData sttPointLinks = sttHitsLinks->GetLinksWithType(sttPointBranchID);
        FairMCPoint *sttpoint = (FairMCPoint*)ioman->GetCloneOfLinkData(sttPointLinks.GetLink(0));
        
        // Terminate if sttpoint=NULL
        if (sttpoint == 0) {continue;}
        
        // Get mcTrackLinks & Data (TCloneArray) from sttpoint (OR sttPointLinks?)
        FairMultiLinkedData mcTrackLinks = sttpoint->GetLinksWithType(mcTrackBranchID); 
        PndMCTrack *mctrack = (PndMCTrack*)ioman->GetCloneOfLinkData(mcTrackLinks.GetLink(0));
        
        // Terminate if mcTrack=NULL
        if (mctrack == 0) {continue;}
        
        // Terminate if not Primary
        if (!mctrack->IsGeneratorCreated())
            continue;
        
        // Hit Counter (very important in case number of tracks vary per event)
        hit_counter++;
        
        
        /* ------------------------------------------------------------------------
        *                          (1) Event Hits
        *  --------------------------------------------------------------------- */
        
        // First Get access to SttHit & PndSttTube
        PndSttHit* stthit = (PndSttHit*)fSttHitArray->At(hit_idx);
        PndSttTube *tube = (PndSttTube*) fTubeArray->At(stthit->GetTubeID());
        
        // Write to xxx-hits.csv
        fHits << hit_counter            << ","      // hit_id
              << stthit->GetX()         << ","      // x-position
              << stthit->GetY()         << ","      // y-position
              << stthit->GetZ()         << ","      // z-position
              << stthit->GetDetectorID()<< ","      // volume_id
              << tube->GetLayerID()     << ","      // layer_id
              << stthit->GetTubeID()                // tube_id/module_id
              << std::endl;
        
        
        /* ------------------------------------------------------------------------
        *                          (2) Event Truths
        *  --------------------------------------------------------------------- */
        
        // Write to xxx-truth.csv
        fTruths << hit_counter       << ","   // hit_id  
                << sttpoint->GetX()  << ","   // tx = true x
                << sttpoint->GetY()  << ","   // ty = true y
                << sttpoint->GetZ()  << ","   // tz = true z
                << sttpoint->GetPx() << ","   // tpx = true px
                << sttpoint->GetPy() << ","   // tpy = true py
                << sttpoint->GetPz() << ","   // tpz = true pz
                << (1.0)             << ",";  // FIXME: weight (sum of weights of hits in track == 1)
        
        
        // Get MCTrack ID associated with each hit_idx (fSttHitArray)
        std::vector<FairLink> mcTrackLinks_sorted = sttHitsLinks->GetSortedMCTracks();
        //std::cout << "No. of MCTracks Linked to this SttHit: " << mcTrackLinks_sorted.size() << std::endl;
        
        for (unsigned int trackIndex = 0; trackIndex < mcTrackLinks_sorted.size(); trackIndex++) {
            
            // Append "particle_id" to xxx-truth.csv
            fTruths << std::to_string(mcTrackLinks_sorted[trackIndex].GetIndex() + 1);
        }
        
        fTruths << std::endl;
        
        /* ------------------------------------------------------------------------
        *                          (3) Event Tubes
        *  --------------------------------------------------------------------- */

        // Write to xxx-cells.csv
        fTubes  << hit_counter             << ","   // hit_id
                << stthit->GetIsochrone()  << ","   // isochrone radius
                << stthit->GetDepCharge()  << ","   // deposited charge
                << stthit->GetEnergyLoss() << ","   // energy loss
                << stthit->GetDetectorID() << ","   // volume_id
                << tube->GetLayerID()      << ","   // layer_id
                << stthit->GetTubeID()     << ","   // tube_id/module_id
                << tube->IsSkew()          << ","   // if tube_id is skewed.
                << tube->GetSectorID()              // in which sector tube exists.
                << std::endl;
        
        /* ------------------------------------------------------------------------
        *                          (4) Event Particles
        *  --------------------------------------------------------------------- */
        
        /** -----------------------------------------------------------------------
        * FIXME: The issue is fParticles should contain unique particle_ids (tracks)
        * e.g. if there are 10 particles/event then it should contain only 10 records.
        * But right now fParticles contain a particle records which are equal to the
        * size of fSttHitArray. Which mean there will be repition of data in the file.
        * One solution might be to write the fParticles outside of fSttHitArray. So I
        * shifted the this piece of code outside of fSttHitArray loop. Note, however, 
        * that if MVD and GEM are also included then fParticles should be generated in 
        * Exec() rather than GenerateMvdData()/GenerateGemData() or GenerateSttData()
        * ---------------------------------------------------------------------- */
        
        //std::cout << "Number of MCTracks (fParticles): " << mcTrackLinks_sorted.size() << std::endl;
        //std::vector<FairLink> mcTrackLinks_sorted = sttHitsLinks->GetSortedMCTracks();
        /*
        // Write to xxx-particles.csv
        for (unsigned int trackIndex=0; trackIndex < mcTrackLinks_sorted.size(); trackIndex++) {
        
            // Fetch mcTrack associated with SttHit
            PndMCTrack *mcTrack = (PndMCTrack*)ioman->GetCloneOfLinkData(mcTrackLinks_sorted[trackIndex]);
            
            // CSV:: Writting Info to CSV File. 
            fParticles  << std::to_string(mcTrackLinks_sorted[trackIndex].GetIndex() + 1)   
                                                                  << ","   // track_id > 0
                        << (mcTrack->GetStartVertex()).X()        << ","   // vx = start x
                        << (mcTrack->GetStartVertex()).Y()        << ","   // vy = start y
                        << (mcTrack->GetStartVertex()).Z()        << ","   // vz = start z
                        << (mcTrack->GetMomentum()).X()           << ","   // px = x-component of track momentum
                        << (mcTrack->GetMomentum()).Y()           << ","   // py = y-component of track momentum
                        << (mcTrack->GetMomentum()).Z()           << ","   // pz = z-component of track momentum
                        << ((mcTrack->GetPdgCode()>0)?1:-1)       << ","   // q = charge of mu-/mu+
                        << mcTrack->GetNPoints(DetectorId::kSTT)  << ","   // FIXME: nhits in STT (Not tested yet).
                        << mcTrack->GetPdgCode()                  << ","   // pdgcode e.g. mu- has pdgcode=-13
                        << mcTrack->GetStartTime()                         // start_time = starting time of particle track
                        << std::endl;
                        
            delete (mcTrack);
        }
        */
    }//end-fSttHitArray
    
    
    /* ------------------------------------------------------------------------
    *                          (4) Event Particles
    *  --------------------------------------------------------------------- */
    
    // TODO: For MCTrack only in STT. I use IdealTrackFinder to access MCTracks
    // in STT using FairLinks, one can also get nhits == linksSTT.GetNLinks()
    
    
    FairMultiLinkedData linksMC, linksSTT;
    FairMultiLinkedData linksMVDPixel,linksMVDStrip,linksGEM,linksSTT;

    
    std::cout << "\nStarting fParticle with number of tracks: " << fBarrelTrackArray->GetEntries() << std::endl;
    
    // Get FairRootManager Instance
    FairRootManager *ioman = FairRootManager::Instance();
    
    // Loop over ideal tracks i.e. BarrelTrackArray
    for (Int_t idx = 0; idx < fBarrelTrackArray->GetEntries(); idx++) { //loop over trackarray
        std::cout << "IdealTrack # : " << idx << std::endl;
        
        // Fetch a PndTrack from the fBarrelTrackArray
        PndTrack *barrelTrack = (PndTrack *)fBarrelTrackArray->At(idx);
        
        // Create the links between the BarrelTrack and the MCTrack
        linksMC = barrelTrack->GetLinksWithType(ioman->GetBranchId("MCTrack")); 
        
        // Here, linksMC.GetNLinks()==1 always.
        if (linksMC.GetNLinks()>0) {
            for (Int_t i=0; i<linksMC.GetNLinks(); i++) {
                if (linksMC.GetLink(i).GetIndex()==barrelTrack->GetTrackCand().getMcTrackId()) {

                    PndMCTrack *mcTrack = (PndMCTrack *)ioman->GetCloneOfLinkData(linksMC.GetLink(i));
                    
                    // Get Only Primary Tracks
                    if (mcTrack->IsGeneratorCreated()) {

                        // Links of Primary Tracks
                        //linksMVDPixel = barrelTrack->GetLinksWithType(ioman->GetBranchId("MvdHitsPixel"));
                        //linksMVDStrip = barrelTrack->GetLinksWithType(ioman->GetBranchId("MvdHitsPixel"));
                        //linksGEM = barrelTrack->GetLinksWithType(ioman->GetBranchId("GEMHit"));
                        linksSTT = barrelTrack->GetLinksWithType(ioman->GetBranchId("STTHit"));
                        
                        // If the number of STT hits greater than 0, write MC track to file!! if linksSTT.GetNLinks() > 0

                        // CSV:: Writting Info to CSV File.
                        fParticles  << (std::to_string(linksMC.GetLink(i).GetIndex() + 1)) << "," // track_id > 0
                                    << (mcTrack->GetStartVertex()).X() << ","   // vx = start x [cm, ns]
                                    << (mcTrack->GetStartVertex()).Y() << ","   // vy = start y [cm, ns]
                                    << (mcTrack->GetStartVertex()).Z() << ","   // vz = start z [cm, ns]
                                    << (mcTrack->GetMomentum()).X()    << ","   // px = x-component of track momentum
                                    << (mcTrack->GetMomentum()).Y()    << ","   // py = y-component of track momentum
                                    << (mcTrack->GetMomentum()).Z()    << ","   // pz = z-component of track momentum
                                    << ((mcTrack->GetPdgCode()>0)?1:-1)<< ","   // q = charge of mu-/mu+
                                    << (linksSTT.GetNLinks())          << ","   // nhits in STT
                                    << mcTrack->GetPdgCode()           << ","   // pdgcode e.g. mu- has pdgcode=-13
                                    << mcTrack->GetStartTime()                  // start_time = starting time of particle track
                                    << std::endl;
                                    
                        }//end-IsGeneratorCreated()
                        
                    }//end-if(GetLink(i))
                    
                }//end-for(GetNLinks)
            }//end-if(GetNLinks)
    }//end-for (barrelTrack)
    
    
    /*
    // Old Solution (Adeel)
    // Write to xxx-particles.csv
    for (Int_t mc=0; mc < fMCTrackArray->GetEntries(); mc++) {
        
        PndMCTrack *mcTrack = (PndMCTrack*)fMCTrackArray->At(mc);
        if (mcTrack->IsGeneratorCreated()) {
            
            // Print MCTrack
            // mcTrack->Print(mc);
            // std::cout << "mcTrack->GetNPoints(kSTT): " << mcTrack->GetNPoints(DetectorID::kSTT) << std::endl;
            
            // CSV:: Writting Info to CSV File. 
            fParticles  << std::to_string(mc + 1)                  << ","   // track_id > 0
                        << (mcTrack->GetStartVertex()).X()         << ","   // vx = start x [cm, ns]
                        << (mcTrack->GetStartVertex()).Y()         << ","   // vy = start y [cm, ns]
                        << (mcTrack->GetStartVertex()).Z()         << ","   // vz = start z [cm, ns]
                        << (mcTrack->GetMomentum()).X()            << ","   // px = x-component of track momentum
                        << (mcTrack->GetMomentum()).Y()            << ","   // py = y-component of track momentum
                        << (mcTrack->GetMomentum()).Z()            << ","   // pz = z-component of track momentum
                        << ((mcTrack->GetPdgCode()>0)?1:-1)        << ","   // q = charge of mu-/mu+
                        //<< mcTrack->GetNPoints(DetectorID::kSTT) << ","   // FIXME: nhits (Not tested yet).
                        << 26                                      << ","   // FIXME: nhits==26 (Just assume)
                        << mcTrack->GetPdgCode()                   << ","   // pdgcode e.g. mu- has pdgcode=-13
                        << mcTrack->GetStartTime()                          // start_time = starting time of particle track
                        << std::endl;
                        
        }//endif-IsGeneratorCreated()
    }//end-for-fMCTrackArray
    */
    
}//end-GenerateSttData()


/* FinishTask() */
void PndMLTracking::FinishTask() {
    
    // Close all files
    fHits.close();
    fTruths.close();
    fParticles.close();
    fTubes.close();
    
    std::cout << "-I- Task Generating CSVs has Finished." << std::endl;
}


/* GetFairMCPoint() */
FairMCPoint* PndMLTracking::GetFairMCPoint(TString fBranchName, FairMultiLinkedData_Interface* links, FairMultiLinkedData& array) {
    
    // get the mc point(s) from each reco hit
    FairMultiLinkedData mcpoints = links->GetLinksWithType(FairRootManager::Instance()->GetBranchId(fBranchName));
    array = mcpoints;
    if (array.GetNLinks() == 0){
        return 0;
    }
    return (FairMCPoint*) FairRootManager::Instance()->GetCloneOfLinkData(array.GetLink(0));
}


/* OLD Code (Kept for Reference) */
/*
// GenerateData()
void PndMLTracking::GenerateData() {
    
    // Event Files
    std::stringstream ss;
    ss << std::setw(10) << std::setfill('0') << fEventId;
    std::string fidx = ss.str();
    //std::cout << "Processing Event: " << "event"+fidx << std::endl;

    // Open Files (CSV)
    fHits.open(fCsvFilesPath+"event"+fidx+"-hits.csv");
    fTruths.open(fCsvFilesPath+"event"+fidx+"-truth.csv");
    fParticles.open(fCsvFilesPath+"event"+fidx+"-particles.csv");
    fTubes.open(fCsvFilesPath+"event"+fidx+"-tubes.csv");

    // ////////////////////////////////////////////////////////////////////
    //                            Add Header                             //
    // ////////////////////////////////////////////////////////////////////
    // Hit Positions
    fHits << "hit_id" <<","
          << "x" << "," 
          << "y" << "," 
          << "z" << ","
          << "tube_id" << "," 
          << "skewed" << "," 
          << "layer_id" << "," 
          << "sector_id" << "," 
          << "volume_id" << "," 
          << "isochrone" << "," 
          << "depcharge" << "," 
          << "energyloss" << "," 
          << "particle_id" 
          <<std::endl;

    // MC Truth
    fTruths << "hit_id" <<","
            << "tx" << ","
            << "ty" << ","
            << "tz" << ","
            << "tpx"<< ","
            << "tpy"<< ","
            << "tpz"<< ","
            // << "weight" << ","
            << "particle_id"
            << std::endl;

    // Particle initial position/vertex(mm), momentum(GeV/c) and charge (q)
    fParticles << "particle_id" <<","
               << "vx" << ","
               << "vy" << ","
               << "vz" << ","
               << "px" << ","
               << "py" << ","
               << "pz" << ","
               << "q"  << ","
               // << "nhits" << ","
               << "pdg_code" << ","
               << "start_time"
               << std::endl;
               
    // Tube Parameters
    // Future Add: GetHalfLength(), GetWireDirection(), GetRadIn(), GetRadOut(),
    //             GetNeighborings(), GetDistance(), IsParallel()
    // Remove Tube Parameters  (redundent w.r.t fHits)
    fTubes << "hit_id"    << ","
           << "tube_id"   << "," 
           << "skewed"    << "," 
           << "layer_id"  << "," 
           << "sector_id" << "," 
           << "volume_id" << "," 
           << "isochrone" << "," 
           << "depcharge" << "," 
           << "energyloss"
           <<std::endl;

    // ////////////////////////////////////////////////////////////////////
    //                            Event Loop
    // ////////////////////////////////////////////////////////////////////
    int hit_id = 0;
    for (int hit_idx = 0; hit_idx < fSttHitArray->GetEntriesFast(); hit_idx++) {

        // Get FairRootManager Instance
        FairRootManager *ioman = FairRootManager::Instance();

        // Get sttHitsLinks from STTHit
        FairMultiLinkedData_Interface *sttHitsLinks = (FairMultiLinkedData_Interface*)fSttHitArray->At(hit_idx);

        // Get sttPointLinks & Data (TCloneArray) from sttHitsLinks
        FairMultiLinkedData sttPointLinks = sttHitsLinks->GetLinksWithType(sttPointBranchID);
        FairMCPoint *sttpoint = (FairMCPoint*)ioman->GetCloneOfLinkData(sttPointLinks.GetLink(0));
        
        // Terminate if sttpoint=NULL
        if (sttpoint == 0) {continue;}

        // Get mcTrackLinks & Data (TCloneArray) from sttpoint (OR sttPointLinks?)
        FairMultiLinkedData mcTrackLinks = sttpoint->GetLinksWithType(mcTrackBranchID); 
        PndMCTrack *mctrack = (PndMCTrack*)ioman->GetCloneOfLinkData(mcTrackLinks.GetLink(0));

        // Terminate if mcTrack=NULL
        if (mctrack == 0) {continue;}

        // Terminate if not Primary
        if (!mctrack->IsGeneratorCreated())
            continue;
        
        // Hit Counter (very important in case number of tracks vary per event)
        hit_id++;
        
        // --------------------------------------------------------------------
        //                            MCPoints/fTruths
        // --------------------------------------------------------------------
        fTruths << hit_id << ","            // hit_id=0 is for interaction vertex
                << sttpoint->GetX()    << ","
                << sttpoint->GetY() << ","
                  << sttpoint->GetZ() << ","
                  << sttpoint->GetPx()<< ","
                  << sttpoint->GetPy()<< ","
                  << sttpoint->GetPz()<< ",";

        // --------------------------------------------------------------------
        //                            STTHists/fHits
        // --------------------------------------------------------------------
        // First Get access to SttHit & PndSttTube
        PndSttHit* stthit = (PndSttHit*)fSttHitArray->At(hit_idx);
        PndSttTube *tube = (PndSttTube*) fTubeArray->At(stthit->GetTubeID());
        fHits << hit_id                 <<","
              << stthit->GetX()         << "," 
              << stthit->GetY()         << "," 
              << stthit->GetZ()         << ","
              << stthit->GetTubeID()    << "," 
              << tube->IsSkew()         << "," 
              << tube->GetLayerID()     << ","
              << tube->GetSectorID()    << "," 
              << stthit->GetDetectorID()<< "," 
              << stthit->GetIsochrone() << "," 
              << stthit->GetDepCharge() << "," 
              << stthit->GetEnergyLoss()<< ",";

        // --------------------------------------------------------------------
        //                            PndTube/fTubes
        // --------------------------------------------------------------------
        // Future Add: GetHalfLength(), GetWireDirection(), GetRadIn(), GetRadOut(),
        //             GetNeighborings(), GetDistance(), IsParallel()
        // Remove Tube Parameters  (redundent w.r.t fHits)
        fTubes<< hit_id                 << ","
              << stthit->GetTubeID()    << ","
              << tube->IsSkew()         << ","
              << tube->GetLayerID()     << ","
              << tube->GetSectorID()    << ","
              << stthit->GetDetectorID()<< "," 
              << stthit->GetIsochrone() << ","
              << stthit->GetDepCharge() << "," 
              << stthit->GetEnergyLoss();

        // Particle/Track initial position/vertex(mm), momentum(GeV/c) and charge (q)
        
        std::vector<FairLink> mcTrackLinks_sorted = sttHitsLinks->GetSortedMCTracks();
        for (unsigned int trackIndex = 0; trackIndex < mcTrackLinks_sorted.size(); trackIndex++) {
            
            // 
            PndMCTrack *myTrack = (PndMCTrack*)ioman->GetCloneOfLinkData(mcTrackLinks_sorted[trackIndex]);
            
            // Get Track Index of MCTrack >> fHits            
            fHits     << std::to_string(mcTrackLinks_sorted[trackIndex].GetIndex() + 1);      // particle_id into hits.csv
            fTruths << std::to_string(mcTrackLinks_sorted[trackIndex].GetIndex() + 1);        // particle_id into truths.csv
            
            // ----------------------------------------------------------------
            //                            fParticles
            // ----------------------------------------------------------------              
            fParticles  << std::to_string(mcTrackLinks_sorted[trackIndex].GetIndex() + 1) << ","
                        << (myTrack->GetStartVertex()).X() << ","
                        << (myTrack->GetStartVertex()).Y() << ","
                        << (myTrack->GetStartVertex()).Z() << ","
                        << (myTrack->GetMomentum()).X() << ","
                        << (myTrack->GetMomentum()).Y() << ","
                        << (myTrack->GetMomentum()).Z() << ","
                        << (-1) << ","                                   // muon
                        << myTrack->GetPdgCode() << ","
                        << myTrack->GetStartTime();
                                  
            delete (myTrack);
        }
        
        // Add Line Breaks
        fHits << std::endl;
        fTruths << std::endl;
        fParticles << std::endl;
        fTubes << std::endl;
        
    }//end-for
    
    // Close Files
    fHits.close();
    fTruths.close();
    fParticles.close();
    fTubes.close();

    fEventId++;
}

// GenerateCSV()
void PndMLTracking::GenerateCSV() {
    
    // Call inside Exec (Waleed)
    InFile1.open (fCsvFilesPath+"box_detector_"+std::to_string(fEventId)+".csv");
    InFile2.open (fCsvFilesPath+"box_real_"+std::to_string(fEventId)+".csv");
    InFile3.open (fCsvFilesPath+"box_mctruth_"+std::to_string(fEventId)+".csv");

    InFile1 << "hit_id" <<","<< "x" <<","<< "y" <<","<< "z"<<","
            << "tube_id" <<","<< "skewed"<<","<< "layer_id"<<","<< "sector_id"<<","
            << "isochrone" <<","<< "depcharge"<<","<< "particle_id"<<std::endl;
           
    InFile2 << "hit_id" <<","<< "tx" <<","<< "ty" <<","<< "tz" <<","<< "px" <<","<< "py"<<","<<"pz"<<std::endl;
    InFile3 << "hit_id" <<","<< "pdgcode" <<","<< "vertexX" <<","<< "vertexY" <<","<< "vertexZ" <<","<< "vertexT"<<std::endl;

    for (int i = 0; i < fSttHitArray->GetEntriesFast(); i++)
    {
        fHitId++;
        
        // Get access to MCPoint >> STTPoint using FairMultiLinkedData_Interface
        FairMultiLinkedData_Interface* myData = (FairMultiLinkedData_Interface*)fSttHitArray->At(i);
        FairMultiLinkedData array;
        FairMCPoint *sttpoint = GetFairMCPoint("STTPoint", myData, array);
        if (sttpoint == 0) {continue;}
        
        InFile2 << fHitId  <<","
                << sttpoint->GetX() <<","
                << sttpoint->GetY() <<","
                  << sttpoint->GetZ() <<","
                  << sttpoint->GetPx() <<","
                  << sttpoint->GetPy() <<","
                  << sttpoint->GetPz();
        
        // Get access to SttHit & PndSttTube
        PndSttHit* stthit = (PndSttHit*)fSttHitArray->At(i);
        PndSttTube *tube = (PndSttTube*) fTubeArray->At(stthit->GetTubeID());
        
        InFile1 << fHitId <<","
                << stthit->GetX() <<","
                << stthit->GetY() <<","
                << stthit->GetZ() <<","
                << stthit->GetTubeID() <<","
                << tube->IsSkew() << ","
                << tube->GetLayerID() << ","
                << tube->GetSectorID() << ","
                << stthit->GetIsochrone() <<","
                << stthit->GetDepCharge() <<",";
        
        std::vector<FairLink> myLinks = myData->GetSortedMCTracks();
        for (unsigned int trackIndex = 0; trackIndex < myLinks.size(); trackIndex++)
        {
            PndMCTrack* myTrack = (PndMCTrack*)FairRootManager::Instance()->GetCloneOfLinkData(myLinks[trackIndex]);
            
            // Get Track Index of MCTrack >> InFile1                
            InFile1 << std::to_string(fEventId)+std::to_string(myLinks[trackIndex].GetIndex());
            
            InFile3 << fHitId <<","
                    <<myTrack->GetPdgCode()<<","
                    <<(myTrack->GetMomentum()).X()<<"," //(myTrack->GetStartVertex()).X()
                    <<(myTrack->GetMomentum()).Y()<<"," //(myTrack->GetStartVertex()).Y()
                    <<(myTrack->GetMomentum()).Z()<<"," //(myTrack->GetStartVertex()).Z()
                    <<myTrack->GetStartTime();
                                         
            delete (myTrack);
        }
        
        InFile1<<std::endl;
        InFile2<<std::endl;
        InFile3<<std::endl;
        
    }//end-for
    
    InFile1.close();
    InFile2.close();
    InFile3.close();
    fEventId++;
}//end-GenerateCSV()
*/

