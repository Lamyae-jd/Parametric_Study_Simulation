/*----------------------
   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/


#ifndef GateTrigger_h
#define GateTrigger_h 1

#include "globals.hh"
#include <iostream>
#include <fstream>
#include <vector>
#include "G4ThreeVector.hh"

#include "GateVPulseProcessor.hh"
#include "GateObjectStore.hh"


class GateTriggerMessenger;


/*! \class  GateTrigger
    \brief  Pulse-processor modeling a trigger received from a pulsed source to calculate time-of-flight.

    - GateTrigger - by julien.rossignol@usherbrooke.ca (October 2020)

    - The operator chooses the depth at which the trigger is apply, the deterministic delay and random jitter.

    \sa GateVPulseProcessor
*/
class GateTrigger : public GateVPulseProcessor
{
  public:

    //! Constructs a new trigger
    GateTrigger(GatePulseProcessorChain* itsChain,
			       const G4String& itsName,G4double itsDelay,
			       G4double itsJitter, G4double itsFrequency) ;
    //! Destructor
    virtual ~GateTrigger() ;

    //! Check the validity of the volume name where the trigger will be applied
    void CheckVolumeName(G4String val);

    //! Returns the deterministicDelay and random jitter
    inline G4double GetRandomJitter()   	      { return m_jitter; }
    inline G4double GetDeterministicDelay()   	      { return m_delay; }
    inline G4double GetFrequency()   	      { return 1/m_period; }
   
    //! Set the jitter and frequency
    inline void SetRandomJitter(G4double val)   { m_jitter = val;  }
    inline void SetDeterministicDelay(G4double val)   { m_delay = val;  }
    inline void SetFrequency(G4double val)   { m_period = 1/val;  }

    //! Implementation of the pure virtual method declared by the base class GateDigitizerComponent
    //! print-out the attributes specific of the timeResolutioner
    virtual void DescribeMyself(size_t indent);



  protected:
    //! Implementation of the pure virtual method declared by the base class GateVPulseProcessor
    //! This methods processes one input-pulse
    //! It is is called by ProcessPulseList() for each of the input pulses
    //! The result of the pulse-processing is incorporated into the output pulse-list
    void ProcessOnePulse(const GatePulse* inputPulse,GatePulseList&  outputPulseList);

  //! To summarize it finds the number of elements of the different scanner levels
  void FindLevelsParams(GateObjectStore* anInserterStore);

  private:
   G4String m_volumeName;  //!< Name of the volume where Dead time is applied
    G4int m_testVolume;     //!< equal to 1 if the volume name is valid, 0 else
    G4int *numberOfComponentForLevel; //!< Table of number of element for each geometric level
    G4int numberOfHigherLevels ;  //!< number of geometric level higher than the one chosen by the user
    G4double m_jitter;     	      	      //!< Random jitter value
    G4double m_delay;     	      	      //!< Deterministic delay value
    G4double m_period;     	      	  //!< Period of the trigger
    unsigned long long int *m_CurrentPulse; //!< Current pulse being evaluated for each volume at given depth.
    GateTriggerMessenger *m_messenger;    //!< Messenger
    G4double *m_triggerTimeTable;  //!< contains the trigger time one for each volume at given depth. Alocated once at the first call.
    static const G4String& theTypeName;   //!< Default type-name
};


#endif
