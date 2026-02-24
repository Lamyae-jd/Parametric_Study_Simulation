/*----------------------
   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/

#include "GateConfiguration.h"

#ifndef GATESCATTERINFORMATIONACTORMESSENGER_HH
#define GATESCATTERINFORMATIONACTORMESSENGER_HH

#include "globals.hh"
#include "GateScatterInformationActor.hh"
#include "GateActorMessenger.hh"
#include "GateUIcmdWith2Vector.hh"
#include "G4UIcmdWithAnInteger.hh"

class GateScatterInformationActor;
class GateScatterInformationActorMessenger: public GateActorMessenger
{
public:
  GateScatterInformationActorMessenger(GateScatterInformationActor* sensor);
  virtual ~GateScatterInformationActorMessenger();

  //NOTE: we keep the messenger member functions just in case we want to add new options to the actor
  void BuildCommands(G4String);
  void SetNewValue(G4UIcommand*, G4String);

protected:
  GateScatterInformationActor * pScatterActor;

  /// Command objects
  G4UIcmdWithAnInteger * pZCutoffmm;
};

#endif /* end #define GATESCATTERINFORMATIONACTORMESSENGER_HH*/
