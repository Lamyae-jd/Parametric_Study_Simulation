/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


#ifndef GATESOURCEVOXELLIZEDPULSED_H
#define GATESOURCEVOXELLIZEDPULSED_H 1

#include "globals.hh"
#include "GateVSource.hh"
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandGamma.h"
#include "GateRandomEngine.hh"
#include "GateSPSTimeDistribution.hh"

class GateSourceVoxellizedPulsedMessenger;
class GateVSourceVoxelReader;

//-----------------------------------------------------------------------------
class GateSourceVoxellizedPulsed : public GateVSource
{
public:

  GateSourceVoxellizedPulsed(G4String name);

  virtual ~GateSourceVoxellizedPulsed();

  virtual G4double GetNextTime(G4double timeNow);

  virtual void Dump(G4int level);

  virtual void Update(G4double time);

  virtual G4int GeneratePrimaries(G4Event* event);

  void ReaderInsert(G4String readerType);

  void ReaderRemove();

  GateVSourceVoxelReader* GetReader() { return m_voxelReader; }

  void          SetPosition(G4ThreeVector pos) { m_sourcePosition = pos; };

  G4ThreeVector GetPosition()                  { return m_sourcePosition; };

  void          SetIsoCenterPosition(G4ThreeVector pos);

  typedef CLHEP::RandGamma RandGamma;
  typedef CLHEP::RandGauss RandGauss;

  void SetJitter(G4double newValue){mJitter = newValue;};
  void SetDelay(G4double newValue){mDelay = newValue;};
  void SetFrequency(G4double newValue){if(newValue != 0) {mPeriod = 1.0/newValue;}};
  void SetAvgPhoton(G4double newValue){mPhotonAvg = newValue;};
  void SetSigmaPhoton(G4double newValue){mPhotonSigma = newValue;};
  void SetTimeFile(G4String newValue){mTimeDistribution->BuildUserSpectrum(newValue);};
  void SetEndTime(G4double newValue){mEndTime = newValue;};

protected:

  GateSourceVoxellizedPulsedMessenger* m_sourceVoxellizedPulsedMessenger;

  // Even if for a standard source the position is controlled by its GPS,
  // for Voxel sources (and maybe in the future for all sources) a position and a
  // rotation are needed to align them to a Geometry Voxel Matrix

  G4ThreeVector                  m_sourcePosition;
  G4RotationMatrix               m_sourceRotation;
  GateVSourceVoxelReader*        m_voxelReader;

  //pulsed source
  double mLastPhoton;
  double mPulseStart;
  G4double mPeriod;
  G4double mJitter;
  G4double mDelay;
  G4double mPhotonAvg;
  G4double mPhotonSigma;
  G4double mTimeK;
  G4double mTimeLambda;
  G4double mEndTime;
  RandGauss * mPulseJitterDistribution;
  RandGauss * mPhotonCountDistribution;
  GateSPSTimeDistribution * mTimeDistribution;
  std::vector<double> * mPhotonList;

};
//-----------------------------------------------------------------------------

#endif
