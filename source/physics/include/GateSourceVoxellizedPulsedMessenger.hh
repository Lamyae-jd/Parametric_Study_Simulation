/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


//      ------------ GateSourceVoxellizedMessenger  ------
//           by G.Santin (18 Dec 2001)
// ************************************************************


#ifndef GATESOURCEVOXELLIZEDPULSEDMESSENGER_H
#define GATESOURCEVOXELLIZEDPULSEDMESSENGER_H 1

#include "globals.hh"
#include "G4UImessenger.hh"
#include "GateMessenger.hh"
#include "GateUIcmdWithAVector.hh"

class GateSourceVoxellizedPulsed;
class GateClock;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithoutParameter;

//-----------------------------------------------------------------------------
class GateSourceVoxellizedPulsedMessenger: public GateMessenger
{
public:
  GateSourceVoxellizedPulsedMessenger(GateSourceVoxellizedPulsed* source);
  ~GateSourceVoxellizedPulsedMessenger();

  virtual void SetNewValue(G4UIcommand*, G4String);

protected:
  GateSourceVoxellizedPulsed*               m_source;
  G4UIcmdWith3VectorAndUnit*          PositionCmd;
  G4UIcmdWith3VectorAndUnit*          translateIsoCenterCmd;
  GateUIcmdWithAVector<G4String>*     ReaderInsertCmd;
  G4UIcmdWithoutParameter*            ReaderRemoveCmd;

  //pulsed
  G4UIcmdWithADoubleAndUnit * pJitterCmd; 
  G4UIcmdWithADoubleAndUnit * pDelayCmd;
  G4UIcmdWithADoubleAndUnit * pFrequencyCmd;
  G4UIcmdWithADouble * pAvgPhotonCmd;
  G4UIcmdWithADouble * pSigmaPhotonCmd;
  G4UIcmdWithAString * pTimeHistCmd;
  G4UIcmdWithADoubleAndUnit * pEndCmd;
};
//-----------------------------------------------------------------------------

#endif
