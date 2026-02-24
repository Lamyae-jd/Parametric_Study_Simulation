#ifndef GATE_GATESPAD_H
#define GATE_GATESPAD_H


#include <G4HCofThisEvent.hh>
#include "GateVActor.hh"
#include "GateSPADMessenger.hh"
#include "isimulator.hh"
#include "root_custom_exporter.hh"
#include "root_simulation_exporter.hh"

class GateSPAD : public GateVActor{
public:
    virtual void SaveData();
    virtual void ResetData();
    virtual void Construct();

    virtual void Initialize(G4HCofThisEvent* ){}
    virtual void EndOfEvent(G4HCofThisEvent*){}
    virtual ~GateSPAD();

    GateSPAD(G4String name, G4int depth=0);
    FCT_FOR_AUTO_CREATOR_ACTOR(GateSPAD)



    virtual void BeginOfRunAction(const G4Run * r);
    virtual void EndOfRunAction(const G4Run * r);
    virtual void BeginOfEventAction(const G4Event *) ;
    virtual void UserSteppingAction(const GateVVolume *, const G4Step*);

    virtual void PreUserTrackingAction(const GateVVolume *, const G4Track*);
    //test nov
    virtual void PostUserTrackingAction(const GateVVolume *, const G4Track*);
    virtual void EndOfEventAction(const G4Event*);

    virtual void RecordEndOfAcquisition();

    G4String config_path, export_path;
    bool local_dsas = true; //compute spad for each event instead of for all acquisition time
    double local_dsas_mesure_time = 150; //micro s
protected:
    GateSPADMessenger* pMessenger;
    ISimulator * simulator;
    RootSimulationCustomExporter * exporter;
    RootSimulationExporter * classic_exporter;
    Vector<Vector<Photon*>> spacial_repartition_photon = Vector<Vector<Photon*>>();
    std::map<uint, uint> last_photon;
    Vector<Vector<Avalanche*>> avalanche_history =  Vector<Vector<Avalanche*>>();
    std::vector<double> xray_emission_time = std::vector<double>();
    double detector_w, detector_h;
    double xray_global_time = -1;
    double xray_creation_global_time = -1;
    uint n_pixel = 0;
    uint8_t matrix_spad_size;
    std::map<uint, uint> geometry_id_spad_id_map;
    uint photon_id = 0; //photon_id 0 is used for no relevant particle data
    uint event_id = -1;
    bool has_hitted = false;

};

MAKE_AUTO_CREATOR_ACTOR(SPAD,GateSPAD)
#endif //GATE_GATESPAD_H
