/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/



#ifndef GATEPHASESA_HH
#define GATEPHASESA_HH

#include "GateVActor.hh"
#include "GateImage.hh"
#include "GateTreeFileManager.hh"

struct iaea_header_type;
struct iaea_record_type;

class G4EmCalculator;

class GatePhaseSAMessenger;

//====================================================================
class GatePhaseSA : public GateVActor {
public:

    virtual ~GatePhaseSA();

    //====================================================================
    // This macro initialize the CreatePrototype and CreateInstance
    FCT_FOR_AUTO_CREATOR_ACTOR(GatePhaseSA)

    //====================================================================
    // Constructs the sensor
    virtual void Construct();

    //====================================================================
    // Callbacks
    virtual void UserSteppingAction(const GateVVolume *, const G4Step *);

    virtual void PreUserTrackingAction(const GateVVolume *, const G4Track *);

    virtual void PostUserTrackingAction(const GateVVolume *, const G4Track *);

    virtual void EndOfEventAction(const G4Event *e);

    virtual void BeginOfRunAction(const G4Run *r);

    virtual void RecordEndOfAcquisition();

    //=======================================================
    /// Saves the data collected to the file
    virtual void SaveData();

    virtual void ResetData();

    void InitTree();

    void SetMaskFilename(G4String filename);

    void SetKillParticleFlag(bool b);

protected:
    GatePhaseSA(G4String name, G4int depth = 0);

    G4String mFileType;
    G4int mNevent;

    GateOutputTreeFileManager *mFile;

    GatePhaseSAMessenger *pMessenger;

    int counter1 = 0;
    int counter2 = 0;


};

MAKE_AUTO_CREATOR_ACTOR(PhaseSA, GatePhaseSA)


#endif /* end #define GATESOURCEACTOR_HH */

