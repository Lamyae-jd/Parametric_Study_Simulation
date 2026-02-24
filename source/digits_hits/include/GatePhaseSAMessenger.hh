/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


/*
  \class  GatePhaseSAMessenger
  \author thibault.frisson@creatis.insa-lyon.fr
  laurent.guigues@creatis.insa-lyon.fr
  david.sarrut@creatis.insa-lyon.fr
  pierre.gueth@creatis.insa-lyon.fr
  brent.huisman@creatis.insa-lyon.fr
*/

#ifndef GATEPHASESAMESSENGER_HH
#define GATEPHASESAMESSENGER_HH

#include "globals.hh"
#include "GateActorMessenger.hh"

class G4UIcmdWithABool;

class G4UIcmdWithoutParameter;

class G4UIcmdWithADoubleAndUnit;

class G4UIcmdWithAString;

class G4UIcmdWith3VectorAndUnit;

class GatePhaseSA;

class GatePhaseSAMessenger : public GateActorMessenger {
public:
    GatePhaseSAMessenger(GatePhaseSA *sensor);

    virtual ~GatePhaseSAMessenger();

    void BuildCommands(G4String base);

    virtual void SetNewValue(G4UIcommand *, G4String);

protected:
    GatePhaseSA *pActor;
};

#endif /* end #define GATESOURCEACTORMESSENGER_HH*/

