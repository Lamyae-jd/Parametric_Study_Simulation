/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#ifndef GateSourceFanBeamMessenger_h
#define GateSourceFanBeamMessenger_h 1

#include "GateConfiguration.h"

#include "globals.hh"
#include "G4UImessenger.hh"
#include "GateMessenger.hh"
#include "GateVSourceMessenger.hh"

class GateSourceFanBeam;
class G4UIcmdWithADoubleAndUnit;


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class GateSourceFanBeamMessenger: public GateVSourceMessenger
{
  public:
    GateSourceFanBeamMessenger(GateSourceFanBeam* source);
    ~GateSourceFanBeamMessenger();
    void SetNewValue(G4UIcommand*, G4String);


  private:
    GateSourceFanBeam * pSourceFanBeam;

    G4UIcmdWithADoubleAndUnit * pHalfLargerCmd; 
    G4UIcmdWithADoubleAndUnit * pHalfSmallCmd;
};

#endif
