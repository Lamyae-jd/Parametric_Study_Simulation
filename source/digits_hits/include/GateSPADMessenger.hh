#ifndef GATE_GATESPADMESSENGER_H
#define GATE_GATESPADMESSENGER_H


#include <G4UIcommand.hh>
#include "GateActorMessenger.hh"

class GateSPAD;

class GateSPADMessenger : public GateActorMessenger{
public:

    GateSPADMessenger(GateSPAD * v);
    virtual ~GateSPADMessenger();

    virtual void SetNewValue(G4UIcommand*, G4String);
    void BuildCommands(G4String base);


protected:
    GateSPAD *pActor;
    G4UIcmdWithAString *config_path_command, *export_path_command;
    G4UIcmdWithABool *local_mode_command;

    G4UIcmdWithAnInteger *local_mode_simuation_length_command;
};

#endif //GATE_GATESPAD_H
