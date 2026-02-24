/*----------------------
   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/


#ifndef GateTriggerMessenger_h
#define GateTriggerMessenger_h 1

#include "GatePulseProcessorMessenger.hh"

class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;

class GateTrigger;

class GateTriggerMessenger: public GatePulseProcessorMessenger
{
  public:
    GateTriggerMessenger(GateTrigger* itsTrigger);
    virtual ~GateTriggerMessenger();

    inline void SetNewValue(G4UIcommand* aCommand, G4String aString);

    inline GateTrigger* GetTrigger()
      { return (GateTrigger*) GetPulseProcessor(); }

  private:
  G4UIcmdWithADoubleAndUnit * pJitterCmd; //!< set the random jitter
  G4UIcmdWithADoubleAndUnit * pFrequencyCmd; //!< set the frequency
  G4UIcmdWithADoubleAndUnit * pDelayCmd; //!< set the deterministic delay
  G4UIcmdWithAString   *newVolCmd;        //!< set the geometric level of application
};

#endif
