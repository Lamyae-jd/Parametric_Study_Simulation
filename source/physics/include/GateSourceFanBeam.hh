/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#ifndef GATESOURCEFANBEAM_HH
#define GATESOURCEFANBEAM_HH

#include "GateConfiguration.h"

#include "Randomize.hh"
#include "G4Event.hh"
#include "globals.hh"
#include "GateActions.hh"
#include "GateVSource.hh"
#include "GateSourceFanBeamMessenger.hh"


class GateSourceFanBeam : public GateVSource, G4UImessenger
{
public:

  GateSourceFanBeam(G4String name, bool useMessenger=true);
  ~GateSourceFanBeam();

  void GeneratePrimaryVertex( G4Event* );
  void SetHalfLarge(G4double newValue){mHalfLarge = newValue;};
  void SetHalfSmall(G4double newValue){mHalfSmall = newValue;};
 
protected:
  GateSourceFanBeamMessenger * pMessenger;
  G4double mHalfLarge;
  G4double mHalfSmall;

};

#endif
