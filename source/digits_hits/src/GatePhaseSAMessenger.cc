/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#include "GatePhaseSAMessenger.hh"

#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "GatePhaseSA.hh"


//-----------------------------------------------------------------------------
GatePhaseSAMessenger::GatePhaseSAMessenger(GatePhaseSA *sensor)
        : GateActorMessenger(sensor), pActor(sensor) {
    BuildCommands(baseName + sensor->GetObjectName());
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
GatePhaseSAMessenger::~GatePhaseSAMessenger() {

}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhaseSAMessenger::BuildCommands(G4String base) {

}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhaseSAMessenger::SetNewValue(G4UIcommand *command, G4String param) {

    GateActorMessenger::SetNewValue(command, param);
}
//-----------------------------------------------------------------------------


