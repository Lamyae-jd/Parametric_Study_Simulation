/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#ifndef GATESOURCEPULSED_HH
#define GATESOURCEPULSED_HH

#include "GateConfiguration.h"

#include "G4Event.hh"
#include "globals.hh"
#include "G4UImessenger.hh"
#include <iomanip>
#include <vector>
#include <string>

#include "GateVSource.hh"
#include "GateSourcePulsedMessenger.hh"

#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandGamma.h"
#include "GateRandomEngine.hh"
#include "GateSPSTimeDistribution.hh"

class GateSourcePulsed : public GateVSource, G4UImessenger
{
public:

  GateSourcePulsed(G4String name, bool useMessenger=true);
  ~GateSourcePulsed();

  typedef CLHEP::RandGamma RandGamma;
  typedef CLHEP::RandGauss RandGauss;

  void SetJitter(G4double newValue){mJitter = newValue;};
  void SetDelay(G4double newValue){mDelay = newValue;};
  void SetFrequency(G4double newValue){if(newValue != 0) {mPeriod = 1.0/newValue;}};
  void SetAvgPhoton(G4double newValue){mPhotonAvg = newValue;};
  void SetSigmaPhoton(G4double newValue){mPhotonSigma = newValue;};
  void SetTimeFile(G4String newValue){mTimeDistribution->BuildUserSpectrum(newValue);};
  void SetEndTime(G4double newValue){mEndTime = newValue;};
  
  G4double GetNextTime( G4double timeStart );

protected:
  GateSourcePulsedMessenger* pMessenger;

  //pulsed source parameters
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

#endif
