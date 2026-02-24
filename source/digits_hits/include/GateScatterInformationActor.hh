/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

//-----------------------------------------------------------------------------
/// \class GateScatterInformationActor
//-----------------------------------------------------------------------------

#include "GateConfiguration.h"

#ifndef GATESCATTERINFORMATIONACTOR_HH
#define GATESCATTERINFORMATIONACTOR_HH

#include "globals.hh"
#include "G4String.hh"
#include <iomanip>   
#include <vector>

// Gate 
#include "GateVActor.hh"
#include "GateImage.hh"
#include "GateSourceMgr.hh"
#include "GateVImageVolume.hh"
#include <G4VEmProcess.hh>
#include "GateScatterInformationActorMessenger.hh"

// Geant4
#include <G4VEMDataSet.hh>
#include <G4EmCalculator.hh>
#include <G4VDataSetAlgorithm.hh>
#include <G4LivermoreComptonModel.hh>
#include <G4LogLogInterpolation.hh>
#include <G4CompositeEMDataSet.hh>

// ROOT
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

class GateScatterInformationActorMessenger;

typedef struct{
    int process;
    double x;
    double y;
    double z;
    double momXInit;
    double momYInit;
    double momZInit;
    double momXFin;
    double momYFin;
    double momZFin;
    double energyInit;
    double energyFin;
    double angle;
    int eventID;
} ScatterInfo;

class GateScatterInformationActor : public GateVActor
{
public:
  //-----------------------------------------------------------------------------
  // This macro initialize the CreatePrototype and CreateInstance
  FCT_FOR_AUTO_CREATOR_ACTOR(GateScatterInformationActor)

  GateScatterInformationActor(G4String name, G4int depth=0);
  ~GateScatterInformationActor();

  // Constructs the actor
  virtual void Construct();
  virtual void SaveData();
  virtual void ResetData();

  // Callbacks
  virtual void UserSteppingAction(const GateVVolume *, const G4Step*);

  int GetZCutoff() {return mZCutoff;}

  void SetZCutoff(int v) {mZCutoff = v;}


protected:
  GateScatterInformationActorMessenger * pActorMessenger;

  TFile * pTfile;
  G4String mTreeName;
  ScatterInfo mScatterInfo;  

  TTree * pTree;

  int mZCutoff;
};
//-----------------------------------------------------------------------------

MAKE_AUTO_CREATOR_ACTOR(ScatterInformationActor, GateScatterInformationActor)

#endif /* end #define GATESCATTERINFORMATIONACTOR_HH */
