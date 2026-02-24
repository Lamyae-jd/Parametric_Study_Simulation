/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


// gate
#include "GateConfiguration.h"
#include "GateSourcePulsed.hh"

// g4
#include "G4Proton.hh"
#include "G4Tokenizer.hh"
#include "G4UnitsTable.hh"
#include "GateApplicationMgr.hh"

// std
#include <string>
#include <algorithm>

GateSourcePulsed::GateSourcePulsed(G4String name, bool useMessenger):
  GateVSource(name)
{
  CLHEP::HepRandomEngine *engine = GateRandomEngine::GetInstance()->GetRandomEngine();
  mLastPhoton = 0;
  mPulseStart = 0;
  mJitter = 0;
  mDelay = 0;
  mPhotonAvg = 1;
  mPhotonSigma = 0;
  mEndTime = 1;
  mPhotonList = new std::vector<double>();
  mPhotonCountDistribution = new RandGauss(engine);
  mPulseJitterDistribution = new RandGauss(engine);
  mTimeDistribution = new GateSPSTimeDistribution();
  if (useMessenger) pMessenger = new GateSourcePulsedMessenger(this);
}

//------------------------------------------------------------------------------------------------------
GateSourcePulsed::~GateSourcePulsed()
{
  //delete mPhotonCountDistribution;
  //delete mTimeDistribution;
  //delete mPulseJitterDistribution;
  if(pMessenger != NULL)
    delete pMessenger;
  delete mPhotonList;
}

G4double GateSourcePulsed::GetNextTime( G4double timeStart )
{
    G4cout << "Pulse time emission:" <<  timeStart << "\n";
    return mPeriod;

    double computedTime = 0.0;
    double jitterStart = 0.0;
    double numberOfPhoton = 1.0;
    G4cout << "*****Generating time ****\n" << std::fixed << std::setprecision(9) << "timeStart:" <<  timeStart << "\n";
    
    while(mPhotonList->size() == 0)
    {
         mPulseStart += mFrequency;
         //computedTime = (mPulseStart - timeStart);
         if(mPulseStart > mEndTime)
         {
             return mEndTime;
         }
         jitterStart = mPulseStart ;//+ ((mPulseJitterDistribution->fire(mDelay, mJitter)));
         //numberOfPhoton = 1; //mPhotonCountDistribution->fire(mPhotonAvg,mPhotonSigma);
         for(double i = 1; i > 0.5; i--)
         {
            mPhotonList->push_back(jitterStart+(mTimeDistribution->GenerateOne()));
         }
         std::sort(mPhotonList->begin(), mPhotonList->end());
         std::reverse(mPhotonList->begin(), mPhotonList->end());
    }   
    computedTime = mPhotonList->back();
    //G4cout << "computedTime:" <<  computedTime << "\n";
    mPhotonList->pop_back();
    return G4double(computedTime-timeStart);
}
