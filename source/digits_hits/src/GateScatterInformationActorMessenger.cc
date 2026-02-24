/*----------------------
   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/

#include "GateConfiguration.h"

#ifndef GATESCATTERINFORMATIONACTORMESSENGER_CC
#define GATESCATTERINFORMATIONACTORMESSENGER_CC

#include "GateScatterInformationActorMessenger.hh"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

//-----------------------------------------------------------------------------
GateScatterInformationActorMessenger::GateScatterInformationActorMessenger(GateScatterInformationActor* sensor):
  GateActorMessenger(sensor),pScatterActor(sensor)
{
  BuildCommands(baseName+pActor->GetObjectName());
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
GateScatterInformationActorMessenger::~GateScatterInformationActorMessenger()
{
    delete pZCutoffmm;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//NOTE: We keep the Messenger structures just in case we want to add new options to the actor
void GateScatterInformationActorMessenger::BuildCommands(G4String base)
{
  G4String guidance;
  G4String bb;

  bb = base+"/setZCutoff";
  pZCutoffmm = new G4UIcmdWithAnInteger(bb, this);
  guidance = G4String("Set the maximum z to save compton information in mm to not save compton in detectors");
  pZCutoffmm->SetGuidance(guidance);
  pZCutoffmm->SetParameterName("ZCutoff", false);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void GateScatterInformationActorMessenger::SetNewValue(G4UIcommand* cmd, G4String param)
{
  if(cmd == pZCutoffmm) pScatterActor->SetZCutoff(  pZCutoffmm->GetNewIntValue(param)  ) ;
  GateActorMessenger::SetNewValue(cmd, param);
}
//-----------------------------------------------------------------------------

#endif /* end #define GATESCATTERINFORMATIONACTORMESSENGER_CC */
