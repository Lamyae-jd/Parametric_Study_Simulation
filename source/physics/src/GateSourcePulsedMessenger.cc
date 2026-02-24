/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#include "GateConfiguration.h"

#include "GateSourcePulsedMessenger.hh"
#include "GateSourcePulsed.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAString.hh"

//----------------------------------------------------------------------------------------
  GateSourcePulsedMessenger::GateSourcePulsedMessenger(GateSourcePulsed* source)
: GateVSourceMessenger(source)
{ 
  pSourcePulsed = source;
  G4String cmdName;
  
  cmdName = GetDirectoryName()+"setPulseJitter";
  pJitterCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);

  cmdName = GetDirectoryName()+"setPulseDelay";
  pDelayCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);
  
  cmdName = GetDirectoryName()+"setFrequency";
  pFrequencyCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);
  
  cmdName = GetDirectoryName()+"setPhotonAvg";
  pAvgPhotonCmd = new G4UIcmdWithADouble(cmdName,this);
  cmdName = GetDirectoryName()+"setPhotonSigma";
  pSigmaPhotonCmd = new G4UIcmdWithADouble(cmdName,this);
  
  
  cmdName = GetDirectoryName()+"setTimeHist";
  pTimeHistCmd = new G4UIcmdWithAString(cmdName,this);

  
  cmdName = GetDirectoryName()+"setPulseEnd";
  pEndCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
GateSourcePulsedMessenger::~GateSourcePulsedMessenger()
{
    delete pJitterCmd;
    delete pDelayCmd;
    delete pFrequencyCmd;
    delete pAvgPhotonCmd;
    delete pSigmaPhotonCmd;
    delete pTimeHistCmd;
    delete pEndCmd;
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
void GateSourcePulsedMessenger::SetNewValue(G4UIcommand* command,G4String newValue)
{
  if (command == pJitterCmd) {pSourcePulsed->SetJitter(pJitterCmd->GetNewDoubleValue(newValue)); }
  if (command == pDelayCmd) {pSourcePulsed->SetDelay(pDelayCmd->GetNewDoubleValue(newValue)); }
  if (command == pFrequencyCmd) {pSourcePulsed->SetFrequency(pFrequencyCmd->GetNewDoubleValue(newValue)); }
  if (command == pAvgPhotonCmd) {pSourcePulsed->SetAvgPhoton(pAvgPhotonCmd->GetNewDoubleValue(newValue)); }
  if (command == pSigmaPhotonCmd) {pSourcePulsed->SetSigmaPhoton(pSigmaPhotonCmd->GetNewDoubleValue(newValue)); }
  if (command == pTimeHistCmd) {pSourcePulsed->SetTimeFile(newValue); }
  if (command == pEndCmd) {pSourcePulsed->SetEndTime(pEndCmd->GetNewDoubleValue(newValue)); }
}
