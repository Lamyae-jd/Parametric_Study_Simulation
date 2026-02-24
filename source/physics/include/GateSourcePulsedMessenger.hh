/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#ifndef GateSourcePulsedMessenger_h
#define GateSourcePulsedMessenger_h 1

#include "GateConfiguration.h"

#include "globals.hh"
#include "G4UImessenger.hh"
#include "GateMessenger.hh"
#include "GateVSourceMessenger.hh"

class GateSourcePulsed;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithADouble;
class G4UIcmdWithAString;
class GateVSource;


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class GateSourcePulsedMessenger: public GateVSourceMessenger
{
  public:
    GateSourcePulsedMessenger(GateSourcePulsed* source);
    ~GateSourcePulsedMessenger();
    void SetNewValue(G4UIcommand*, G4String);
  private:
    GateSourcePulsed * pSourcePulsed;
    G4UIcmdWithADoubleAndUnit * pJitterCmd; 
    G4UIcmdWithADoubleAndUnit * pDelayCmd;
    G4UIcmdWithADoubleAndUnit * pFrequencyCmd;
    G4UIcmdWithADouble * pAvgPhotonCmd;
    G4UIcmdWithADouble * pSigmaPhotonCmd;
    G4UIcmdWithAString * pTimeHistCmd;
    G4UIcmdWithADoubleAndUnit * pEndCmd;
};

#endif
