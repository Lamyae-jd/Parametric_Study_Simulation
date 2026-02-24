/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


// gate
#include "GateConfiguration.h"
#include "GateSourceFanBeam.hh"
#include "G4RunManager.hh"

// std
#include <string>

GateSourceFanBeam::GateSourceFanBeam(G4String name, bool useMessenger):
  GateVSource(name)
{
    if (useMessenger) pMessenger = new GateSourceFanBeamMessenger(this);
}

//------------------------------------------------------------------------------------------------------
GateSourceFanBeam::~GateSourceFanBeam()
{
    
}




//------------------------------------------------------------------------------------------------------
void GateSourceFanBeam::GeneratePrimaryVertex( G4Event* aEvent )
{
  if( GetParticleDefinition() == NULL ) return;
  if( GetPosDist()->GetPosDisType() == "UserFluenceImage" ) InitializeUserFluence();
  if( mUserFocalShapeInitialisation ) InitializeUserFocalShape();

  if( nVerboseLevel > 1 ) {
    G4cout << " NumberOfParticlesToBeGenerated: " << GetNumberOfParticles() << Gateendl ;
  }

  /* PY Descourt 08/09/2009 */
  // TrackingMode theMode =( (GateSteppingAction *)(GateRunManager::GetRunManager()->GetUserSteppingAction() ) )->GetMode();
  //if (  theMode == TrackingMode::kBoth || theMode == TrackingMode::kTracker )
    //{
  G4ThreeVector particle_position;
  if(mIsUserFluenceActive) { particle_position = UserFluencePosGenerateOne(); }
  else { particle_position = m_posSPS->GenerateOne(); }

  // Set placement relative to attached volume
  ChangeParticlePositionRelativeToAttachedVolume(particle_position);

  G4PrimaryVertex* vertex = new G4PrimaryVertex(particle_position, GetParticleTime());
  if (GetNumberOfParticles() == 0) {
    GateError("Something went wrong, nb of particle is 0 in GateVSource::GeneratePrimaryVertex\n");
  }

  for( G4int i = 0 ; i != GetNumberOfParticles() ; ++i )
    {
      G4ParticleMomentum particle_momentum_direction;
      G4double angLarge = 2*mHalfLarge*G4UniformRand() - mHalfLarge;             
      G4double angSmall = 2*mHalfSmall*G4UniformRand() - mHalfSmall;
      
      G4double z = 1/std::sqrt(1 + std::pow(std::tan(angLarge),2) + std::pow(std::tan(angSmall),2));
      G4double x = z*std::tan(angLarge);
      G4double y = z*std::tan(angSmall);
      //G4cout << "Large angle: " << angLarge << Gateendl;
      //#G4cout << "Small angle: " << angSmall << Gateendl;
      //G4cout << "X: " << x << Gateendl;
      //G4cout << "Y: " << y << Gateendl;
      //G4cout << "Z: " << z << Gateendl;

      particle_momentum_direction.setX(x);
      particle_momentum_direction.setY(y);
      particle_momentum_direction.setZ(z);

      // Set placement relative to attached volume
      ChangeParticleMomentumRelativeToAttachedVolume(particle_momentum_direction);
      // DD(particle_momentum_direction);

      G4double particle_energy = 0;
      particle_energy = m_eneSPS->GenerateOne( GetParticleDefinition() );
      mEnergy = particle_energy; // because particle_energy is private

      G4double mass =  GetParticleDefinition()->GetPDGMass();
      G4double energy = particle_energy + mass;
      G4double pmom = std::sqrt( energy * energy - mass * mass );
      G4double px = pmom * particle_momentum_direction.x();
      G4double py = pmom * particle_momentum_direction.y();
      G4double pz = pmom * particle_momentum_direction.z();

      G4PrimaryParticle* particle = new G4PrimaryParticle(GetParticleDefinition(), px, py, pz);
      particle->SetMass( mass );
      particle->SetCharge( GetParticleDefinition()->GetPDGCharge() );
      particle->SetPolarization( GetParticlePolarization().x(),
                                  GetParticlePolarization().y(),
                                  GetParticlePolarization().z() );

      G4double particle_weight = GetBiasRndm()->GetBiasWeight();
      particle->SetWeight( particle_weight );

      // Add one particle
      vertex->SetPrimary( particle );

// Verbose
      if( nVerboseLevel > 1 ) {
        G4cout << "Particle name: " << GetParticleDefinition()->GetParticleName() << Gateendl;
        G4cout << "       Energy: " << particle_energy << Gateendl ;
        G4cout << "     Position: " << particle_position << Gateendl ;
        G4cout << "    Direction: " << particle_momentum_direction << Gateendl;
      }
      if( nVerboseLevel > 2 ) {
        G4cout << "Creating primaries and assigning to vertex\n";
      }
    } // end loop on NumberOfParticles

  aEvent->AddPrimaryVertex( vertex );
  //}
  /////  HERE we are in DETECTOR MODE
  /* PY Descourt 08/09/2009 */

  // if ( theMode == TrackingMode::kDetector )
  //   {
  //     // create a new vertex
  //     G4PrimaryVertex* vertex =  new G4PrimaryVertex(G4ThreeVector(0.,0.,0.),GetTime());

  //     if(GetVerboseLevel() > 0)
  //       G4cout << "Creating primaries and assigning to vertex\n";

  //     for( G4int i=0; i<GetNumberOfParticles(); i++ )
  //       {
  //         G4PrimaryParticle* particle = new G4PrimaryParticle(m_pd,0. ,0. , 0.);
  //         vertex->SetPrimary( particle );
  //       }

  //     aEvent->AddPrimaryVertex( vertex );
  //   }
  if( nVerboseLevel > 1 )
    G4cout << " Primary Vertex generated !\n";
}
//-------------------------------------------------------------------------------------------------

