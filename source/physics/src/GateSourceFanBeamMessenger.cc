/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#include "GateConfiguration.h"


#include "GateSourceFanBeamMessenger.hh"
#include "GateSourceFanBeam.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

//----------------------------------------------------------------------------------------
  GateSourceFanBeamMessenger::GateSourceFanBeamMessenger(GateSourceFanBeam* source)
: GateVSourceMessenger(source)
{ 
  pSourceFanBeam = source;
  G4String cmdName;
  
  cmdName = GetDirectoryName()+"setHalfLarge";
  pHalfLargerCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);

  cmdName = GetDirectoryName()+"setHalfSmall";
  pHalfSmallCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
GateSourceFanBeamMessenger::~GateSourceFanBeamMessenger()
{
    delete pHalfLargerCmd;
    delete pHalfSmallCmd;
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
void GateSourceFanBeamMessenger::SetNewValue(G4UIcommand* command,G4String newValue)
{
  if (command == pHalfLargerCmd) {pSourceFanBeam->SetHalfLarge(pHalfLargerCmd->GetNewDoubleValue(newValue)); }
  if (command == pHalfSmallCmd) {pSourceFanBeam->SetHalfSmall(pHalfSmallCmd->GetNewDoubleValue(newValue)); }
}
