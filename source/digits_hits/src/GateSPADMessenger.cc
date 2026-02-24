#include "GateSPADMessenger.hh"
#include "GateSPAD.hh"

#include "GateActorMessenger.hh"
#include "G4UIcmdWithAnInteger.hh"
GateSPADMessenger::~GateSPADMessenger() {

}

GateSPADMessenger::GateSPADMessenger(GateSPAD *v) : GateActorMessenger(v),
                                                    pActor(v){
    G4String base = baseName+pActor->GetObjectName();
    G4String bb = base+"/SPAD_config";
    auto c1 = bb+"/ConfigPath";
    config_path_command = new G4UIcmdWithAString(c1.c_str(), this);
    auto c2 = bb+"/ExportPath";
    export_path_command = new G4UIcmdWithAString(c2.c_str(), this);
    auto c3 = bb+"/LocalMode";
    local_mode_command = new G4UIcmdWithABool(c3.c_str(), this);
    local_mode_command->SetGuidance("localMode simulate SPAD only for the event where a photon touch a SPAD array; huge root size reduction");
    auto c4 = bb+"/LocalModeLength";
    local_mode_simuation_length_command = new G4UIcmdWithAnInteger(c4.c_str(), this);
    local_mode_simuation_length_command->SetGuidance("time of simulation in micro seconds for each photon event on the SPAD");
}


void GateSPADMessenger::SetNewValue(G4UIcommand * c, G4String s)  {
    GateActorMessenger::SetNewValue(c,s);
   if (c == config_path_command){
       pActor->config_path = s;
   }
   else if (c == export_path_command){
        pActor->export_path = s;
    }
   else if (c == local_mode_command){
       pActor->local_dsas = s == "true";
   }
   else if (c == local_mode_simuation_length_command){
       pActor->local_dsas_mesure_time = std::stod(s);
   }

}

void GateSPADMessenger::BuildCommands(G4String) {

}
