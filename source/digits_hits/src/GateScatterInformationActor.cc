/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#include "GateConfiguration.h"

// Gate 
#include "GateScatterInformationActor.hh"
#include "GateMiscFunctions.hh"

// G4
#include <G4Event.hh>
#include <G4MaterialTable.hh>
#include <G4ParticleTable.hh>
#include <G4VEmProcess.hh>
#include <G4TransportationManager.hh>
#include <G4LivermoreComptonModel.hh>
#include <G4EventManager.hh>

//-----------------------------------------------------------------------------

/// Constructors
GateScatterInformationActor::GateScatterInformationActor(G4String name, G4int depth):
  GateVActor(name,depth)
{
  GateMessage("Actor",0,"GateScatterInformationActor() -- begin\n");
  pActorMessenger = new GateScatterInformationActorMessenger(this);
  GateMessage("Actor",0,"GateScatterInformationActor() -- end\n");
  mZCutoff = 10000;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/// Destructor 
GateScatterInformationActor::~GateScatterInformationActor()
{
  delete pActorMessenger;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Construct
void GateScatterInformationActor::Construct()
{
  GateVActor::Construct();
  //  Callbacks
  EnableUserSteppingAction(true);

  //mHistName = "Precise/output/TrackLength.root";
  pTfile = new TFile(mSaveFilename,"RECREATE");
  pTree = new TTree("scattered","Tree of scattered photons information");

  pTree->Branch("process",&mScatterInfo.process,"process/I");  
  pTree->Branch("x",&mScatterInfo.x,"x/D");  
  pTree->Branch("y",&mScatterInfo.y,"y/D");  
  pTree->Branch("z",&mScatterInfo.z,"z/D");  
  pTree->Branch("momXInit",&mScatterInfo.momXInit,"momXInit/D");  
  pTree->Branch("momYInit",&mScatterInfo.momYInit,"momYInit/D");  
  pTree->Branch("momZInit",&mScatterInfo.momZInit,"momZInit/D");  
  pTree->Branch("momXFin",&mScatterInfo.momXFin,"momXFin/D");  
  pTree->Branch("momYFin",&mScatterInfo.momYFin,"momYFin/D");  
  pTree->Branch("momZFin",&mScatterInfo.momZFin,"momZFin/D");  
  pTree->Branch("energyInit",&mScatterInfo.energyInit,"energyInit/D");  
  pTree->Branch("energyFin",&mScatterInfo.energyFin,"energyFin/D");  
  pTree->Branch("angle",&mScatterInfo.angle,"angle/D");  
  pTree->Branch("eventID",&mScatterInfo.eventID,"eventID/I");  
  ResetData();
}
//-----------------------------------------------------------------------------
/// Save data
void GateScatterInformationActor::SaveData()
{
  GateVActor::SaveData();
  pTfile->Write();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GateScatterInformationActor::ResetData()
{
  pTree->Reset();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Callbacks
void GateScatterInformationActor::UserSteppingAction(const GateVVolume* v, const G4Step* step)
{
  GateVActor::UserSteppingAction(v, step);

  const G4VProcess *pr = step->GetPostStepPoint()->GetProcessDefinedStep();
  const G4VEmProcess *process = dynamic_cast<const G4VEmProcess*>(pr);
  if(!process) 
    return;

  if((process->GetProcessName() == G4String("Compton") || process->GetProcessName() == G4String("compt")) && step->GetPostStepPoint()->GetPosition().z() < GetZCutoff()) 
    {
        mScatterInfo.process = 0;
    }
    else if((process->GetProcessName() == G4String("RayleighScattering") || process->GetProcessName() == G4String("Rayl"))&& step->GetPostStepPoint()->GetPosition().z() < GetZCutoff())
    {
        mScatterInfo.process = 1;
    }
    else{return;}


    mScatterInfo.x = step->GetPostStepPoint()->GetPosition().x();
    mScatterInfo.y = step->GetPostStepPoint()->GetPosition().y();
    mScatterInfo.z = step->GetPostStepPoint()->GetPosition().z();
    mScatterInfo.momXInit = step->GetPreStepPoint()->GetMomentumDirection().x();
    mScatterInfo.momYInit = step->GetPreStepPoint()->GetMomentumDirection().y();
    mScatterInfo.momZInit = step->GetPreStepPoint()->GetMomentumDirection().z();
    mScatterInfo.momXFin = step->GetPostStepPoint()->GetMomentumDirection().x();
    mScatterInfo.momYFin = step->GetPostStepPoint()->GetMomentumDirection().y();
    mScatterInfo.momZFin = step->GetPostStepPoint()->GetMomentumDirection().z();
    mScatterInfo.energyInit = step->GetPreStepPoint()->GetTotalEnergy();
    mScatterInfo.energyFin = step->GetPostStepPoint()->GetTotalEnergy();
    mScatterInfo.angle = acos((mScatterInfo.momXInit*mScatterInfo.momXFin)+(mScatterInfo.momYInit*mScatterInfo.momYFin)+(mScatterInfo.momZInit*mScatterInfo.momZFin))*180/3.141592;
    mScatterInfo.eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    pTree->Fill();
}

//-----------------------------------------------------------------------------
