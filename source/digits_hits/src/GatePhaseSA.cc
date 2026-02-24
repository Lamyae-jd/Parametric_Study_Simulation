/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


//
/*
  \brief Class GatePhaseSA
  \author thibault.frisson@creatis.insa-lyon.fr
  laurent.guigues@creatis.insa-lyon.fr
  david.sarrut@creatis.insa-lyon.fr
  brent.huisman@insa-lyon.fr
*/


#include "GatePhaseSA.hh"
#include <G4EmCalculator.hh>
#include <G4VProcess.hh>
#include <G4Run.hh>
#include "GateMiscFunctions.hh"

#include "GateSourceMgr.hh"
#include "GateVImageActor.hh"
#include "GateProtonNuclearInformationActor.hh"

#include "GateSourceTPSPencilBeam.hh"
#include "GatePhaseSAMessenger.hh"
#include "GateIAEAHeader.h"

#include <vector>

// --------------------------------------------------------------------
GatePhaseSA::GatePhaseSA(G4String name, G4int depth) :
        GateVActor(name, depth) {

    pMessenger = new GatePhaseSAMessenger(this);


}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
/// Destructor
GatePhaseSA::~GatePhaseSA() {
    delete pMessenger;
}
// --------------------------------------------------------------------

// --------------------------------------------------------------------
/// Construct
void GatePhaseSA::Construct() {
    GateVActor::Construct();
    // Enable callbacks
    EnableBeginOfRunAction(true);
    EnableBeginOfEventAction(true);
    EnableRecordEndOfAcquisition(true);
    EnablePreUserTrackingAction(true);
    EnablePostUserTrackingAction(true);
    EnableUserSteppingAction(true);
    return;

}
// --------------------------------------------------------------------

void GatePhaseSA::InitTree() {
    G4cout << "are"  << Gateendl;

}

// --------------------------------------------------------------------
void GatePhaseSA::BeginOfRunAction(const G4Run *r) {
    G4cout << "ea"  << Gateendl;
}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::RecordEndOfAcquisition() {
    G4cout << "aa"  << Gateendl;
    return;

}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::SetMaskFilename(G4String filename) {
    G4cout << "as"  << Gateendl;
}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::SetKillParticleFlag(bool b) {
    G4cout << "ad"  << Gateendl;
}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::PreUserTrackingAction(const GateVVolume * /*v*/, const G4Track *t) {
    //G4cout << "ac"  << Gateendl;
    counter1++;

}
// --------------------------------------------------------------------

// --------------------------------------------------------------------
void GatePhaseSA::PostUserTrackingAction(const GateVVolume * /*v*/, const G4Track *t) {
    //G4cout << "av"  << Gateendl;
    counter2++;
}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::EndOfEventAction(const G4Event *e){


    G4cout << counter1 << "  " << counter2  << Gateendl;
    auto fd = e->GetHCofThisEvent();
    auto capa = (int)fd->GetCapacity();
    for (int k=0; k < capa; k++){
        auto hits = fd->GetHC(k);
        //hits->PrintAllHits();
        auto photon_number = hits->GetSize();
        G4cout << "photon_number : " << photon_number << Gateendl;

        for (uint i=0; i < photon_number;i++){
            auto hit = hits->GetHit(i);
            auto v = hit->CreateAttValues();
// printf("phot : %p %p\n",v->begin(),v->end());
        }
        //
            //auto primary_particle = hits->GetHit(i);
            //auto wavelength = c_light * h_Planck / total_energy / nanometer;
            //printf("\n%f\n",wavelength);

        //}
    }
    // Set Primary Energy


}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::UserSteppingAction(const GateVVolume *, const G4Step *step) {
    G4cout << "ads"  << Gateendl;

    auto stepPoint = step->GetPostStepPoint();
    auto energy = stepPoint->GetKineticEnergy();
    auto track = step->GetTrack();
    G4ThreeVector localPosition = stepPoint->GetPosition();
    G4ThreeVector localMomentum = stepPoint->GetMomentumDirection();
    G4VPhysicalVolume *pv = step->GetTrack()->GetNextVolume();

    //step->GetTrack()->SetTrackStatus(fStopAndKill);
    return;

}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::SaveData() {
    return;
}
// --------------------------------------------------------------------


// --------------------------------------------------------------------
void GatePhaseSA::ResetData() {
    GateError("Can't reset phase space");
}
// --------------------------------------------------------------------
