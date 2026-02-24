/*----------------------
   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/


#include "GateTrigger.hh"

#include "G4UnitsTable.hh"

#include "GateTriggerMessenger.hh"
#include "GateTools.hh"

#include "Randomize.hh"
#include "GateConstants.hh"


GateTrigger::GateTrigger(GatePulseProcessorChain* itsChain, const G4String& itsName,G4double itsDelay=0,
			       G4double itsJitter=0., G4double itsFrequency=1)
  : GateVPulseProcessor(itsChain, itsName),
    m_jitter(itsJitter),m_delay(itsDelay),m_period(1/itsFrequency)
{
  m_messenger = new GateTriggerMessenger(this);
}




GateTrigger::~GateTrigger()
{
  delete m_messenger;
}



void GateTrigger::ProcessOnePulse(const GatePulse* inputPulse,GatePulseList& outputPulseList)
{
  if(m_period <= 0 ) {
    G4cerr << 	Gateendl << "[GateTrigger::ProcessOnePulse]:\n"
      	   <<   "Sorry, but the resolution (" << GetFrequency() << ") is invalid\n";
    G4Exception( "GateTemporalResolution::ProcessOnePulse", "ProcessOnePulse", FatalException,
			"You must choose a temporal resolution >= 0 /gate/digitizer/Singles/Singles/timeResolution/setTimeResolution TIME\n or disable the temporal resolution using:\n\t/gate/digitizer/Singles/Singles/timeResolution/disable\n");
  }
  else 
  {
        if (!inputPulse) {
          if (nVerboseLevel>1)
	    G4cout << "[GateTrigger::ProcessOnePulse]: input pulse was null -> nothing to do\n\n";
          return;
        }

        if (inputPulse)
        {
	        GatePulse* outputPulse = new GatePulse(*inputPulse);

            // FIND THE ELEMENT ID OF PULSE
            const GateVolumeID* aVolumeID = &inputPulse->GetVolumeID();
            G4int m_generalDetId = 0; // a unique number for each detector part
                                    // that depends of the depth of application
                                    // of the trigger
            size_t m_depth = (size_t)(aVolumeID->GetCreatorDepth(m_volumeName));

            m_generalDetId = aVolumeID->GetCopyNo(m_depth);

            G4int multFactor = 1;
            for(G4int i = 1 ; i < numberOfHigherLevels + 1; i++)
                {
                  multFactor *= numberOfComponentForLevel[i-1];
                  m_generalDetId += aVolumeID->GetCopyNo(m_depth-i)*multFactor;
                }
            
            //TIME OF THE PULSE
            G4double currentTime = (inputPulse->GetTime());
            unsigned long long int  pulseNumber = (unsigned long long int) currentTime/m_period;

            if(pulseNumber !=  m_CurrentPulse[m_generalDetId])
            {
                m_CurrentPulse[m_generalDetId] = pulseNumber;
                m_triggerTimeTable[m_generalDetId] = pulseNumber*m_period + m_delay + G4RandGauss::shoot(0, m_jitter);
                
            }
            outputPulse->SetTimeOfFlight(currentTime - m_triggerTimeTable[m_generalDetId]);
            outputPulseList.push_back(outputPulse);
        }
	
    }
}

void GateTrigger::CheckVolumeName(G4String val)
{
  GateObjectStore* anInserterStore = GateObjectStore::GetInstance();

  if (anInserterStore->FindCreator(val)) {
    m_volumeName = val;

    FindLevelsParams(anInserterStore);
    m_testVolume = 1;
  }
  else {
    G4cout << "Wrong Volume Name\n";
  }
}



void GateTrigger::FindLevelsParams(GateObjectStore*  anInserterStore)
{


  G4int numberTotalOfComponentInSystem = 0;

  GateVVolume* anInserter = anInserterStore->FindCreator(m_volumeName);
  GateVVolume* anotherInserter = anInserter; // just to buffer anInserter


  if(nVerboseLevel>1)
    G4cout << "TRIGGER IS APPLIED ON " <<  m_volumeName << Gateendl;


  // How many levels higher than volumeName level ?
  numberOfHigherLevels = 0;
  while(anotherInserter->GetMotherList())
    {
      anotherInserter =  anotherInserter->GetMotherList()->GetCreator();
      numberOfHigherLevels ++;
    }
  //  numberOfHigherLevels--;
  anotherInserter = anInserter;
  // G4cout<<"N of Higher level "<<numberOfHigherLevels<< Gateendl;

  // How many components for each levels ?
  numberOfComponentForLevel = (G4int*)malloc(sizeof(G4int)*(numberOfHigherLevels));
  if(!numberOfComponentForLevel)
    {
      G4cout << "[GateTrigger::FindLevelsParams]: new failed\n\n";
      return;
    }


  numberOfComponentForLevel[0] = anotherInserter->GetVolumeNumber();
  

  for(G4int i = 1 ; i < numberOfHigherLevels ; i++)
    {
      anotherInserter = anotherInserter->GetMotherList()->GetCreator();
      numberOfComponentForLevel[i] = anotherInserter->GetVolumeNumber();

    }

  numberTotalOfComponentInSystem = 1;
  for(G4int i2 = 0 ; i2 < numberOfHigherLevels ; i2++)
    {
      numberTotalOfComponentInSystem = numberTotalOfComponentInSystem * numberOfComponentForLevel[i2];
      if(nVerboseLevel>5)
        G4cout << "Level : " << i2 << " has "
               << numberOfComponentForLevel[i2] << " elements\n";
    }

  if(nVerboseLevel>5)
    G4cout << "total number of elements = " <<numberTotalOfComponentInSystem << Gateendl;

  // create the table of pulse and trigger time table
  if(numberTotalOfComponentInSystem > 0)
    {
    m_CurrentPulse = (unsigned long long int*)malloc(sizeof(unsigned long long int)*numberTotalOfComponentInSystem);
    m_triggerTimeTable = (G4double*)malloc(sizeof(G4double)*numberTotalOfComponentInSystem);
    }
}


void GateTrigger::DescribeMyself(size_t indent)
{
  G4cout << GateTools::Indent(indent) << "Trigger: " << G4BestUnit(m_jitter,"Random Jitter") << Gateendl;
}
