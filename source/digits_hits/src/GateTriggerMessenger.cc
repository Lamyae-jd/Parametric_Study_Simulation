/*----------------------
   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/


#include "GateTriggerMessenger.hh"

#include "GateTrigger.hh"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

GateTriggerMessenger::GateTriggerMessenger(GateTrigger* itsTrigger)
    : GatePulseProcessorMessenger(itsTrigger)
{
  G4String guidance;
  G4String cmdName;

  cmdName = GetDirectoryName() + "setJitter";
  pJitterCmd= new G4UIcmdWithADoubleAndUnit(cmdName,this);
  pJitterCmd->SetGuidance("Set random jitter (in ps) of the trigger");
  pJitterCmd->SetUnitCategory("Time");

  cmdName = GetDirectoryName() + "setDelay";
  pDelayCmd= new G4UIcmdWithADoubleAndUnit(cmdName,this);
  pDelayCmd->SetGuidance("Set deterministic delay (in ps) of the trigger");
  pDelayCmd->SetUnitCategory("Time");

  cmdName = GetDirectoryName() + "setFrequency";
  pFrequencyCmd= new G4UIcmdWithADoubleAndUnit(cmdName,this);
  pFrequencyCmd->SetGuidance("Set frequency of the trigger");
  pFrequencyCmd->SetUnitCategory("Frequency");

  cmdName = GetDirectoryName() + "chooseTriggerVolume";
  newVolCmd = new G4UIcmdWithAString(cmdName,this);
  newVolCmd->SetGuidance("Choose a volume (depth) for trigger(e.g. crystal)");
}


GateTriggerMessenger::~GateTriggerMessenger()
{
  delete pJitterCmd;
  delete newVolCmd;
  delete pDelayCmd;
  delete pFrequencyCmd;
}


void GateTriggerMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
  if (command== pJitterCmd)
    { GetTrigger()->SetRandomJitter(pJitterCmd->GetNewDoubleValue(newValue)); }
  else if (command==newVolCmd )
    GetTrigger()->CheckVolumeName(newValue);
  else if (command == pDelayCmd)
    GetTrigger()->SetDeterministicDelay(pDelayCmd->GetNewDoubleValue(newValue));
  else if (command == pFrequencyCmd)
    GetTrigger()->SetFrequency(pFrequencyCmd->GetNewDoubleValue(newValue));
  else
    GatePulseProcessorMessenger::SetNewValue(command,newValue);
}
