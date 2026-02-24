#include "GateSPAD.hh"
#include "GateSPADMessenger.hh"

#include "localMetaConfiguration.h"
#include "position.hh"
#include "photon.hh"
#include "ConfigFile.h"
#include "utils.hh"
#include <string>
#include <G4EventManager.hh>
#include <G4TransportationManager.hh>
#include "avalanche.hh"
#include "GateApplicationMgr.hh"
#include "GateBox.hh"
#include "isensor_matrix.hh"
#include "igenerator.hh"

GateSPAD::GateSPAD(G4String name, G4int depth):
    GateVActor(name,depth) {
    pMessenger = new GateSPADMessenger(this);
}

void GateSPAD::ResetData() {
}

void GateSPAD::SaveData() {
}

void GateSPAD::Construct() {

    if (config_path.empty()){
        GateError("please set the config path of DSAS : /gate/actor/" + mName + "/SPAD_config/ConfigPath [PATH]");
    }

    if (mVolumeName.empty()){
        GateError("please attach a volume to the actor : /gate/actor/"+ mName +"/attachTo [VOLUME NAME]");
    }

    ConfigFile config = ConfigFile(config_path,"=","#","EndConfigFile");
    simulator = getSimulatorFromConfig(config);
    auto sensor = simulator->getSensorMatrix();
    auto sensor_dimension = sensor->getDimension();
    auto box = (GateBox *) mVolume; //TODO : check dynamic cast
    if (box == nullptr)
        GateError("The selected volume is not a Box! Only box can be used as SPAD");
    detector_h = box->GetBoxYLength() / um;
    detector_w = box->GetBoxXLength() / um;
    if  (abs(sensor_dimension.x - detector_w) > 10e-7 or abs(sensor_dimension.y - detector_h) > 10e-7){
        G4cout << "sensor dimension : " << sensor_dimension.x << " x " << sensor_dimension.y << " micro m" << Gateendl;
        G4cout << "geometry dimension : " << detector_w << " x " << detector_h << " micro m" << Gateendl;
        GateError("Please ensure that the sensor config and the simulation geometry fit");
    }

    exporter = new RootSimulationCustomExporter(export_path, "");
    classic_exporter = new RootSimulationExporter(export_path + "modif", "");
    //access world geometry
    G4Navigator* navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
    auto world = navigator->GetWorldVolume()->GetLogicalVolume();
    auto daughter = world->GetDaughter(0)->GetLogicalVolume();

    //auto ds = daughter->T();

    n_pixel = 0;
    for(int i = 0; i < (int)daughter->GetNoDaughters(); i++){
        auto daugter = daughter->GetDaughter(i);
        if (daugter->GetName() == "pixel_phys"){
            geometry_id_spad_id_map[daugter->GetInstanceID()] = n_pixel++;
            spacial_repartition_photon.push_back(Vector<Photon*>());
            auto pos = daugter->GetTranslation(); //TODO : check dynamic cast
            G4cout << pos << G4endl;
        }
    }

    if (floor(sqrt(n_pixel)) * floor(sqrt(n_pixel)) != n_pixel && ceil(sqrt(n_pixel)) * ceil(sqrt(n_pixel)) != n_pixel){ // two needed because of float truncate
        GateError("Please ensure to have a perfect square number of pixel");
    }

    matrix_spad_size = floor(sqrt(n_pixel));

    avalanche_history.clear();
    GateVActor::Construct();
    EnableEndOfRunAction(true);
    EnableUserSteppingAction(true);
    EnableBeginOfEventAction(true);
    EnableRecordEndOfAcquisition(true);
    EnablePreUserTrackingAction(true);
    EnablePostUserTrackingAction(true);
}

void GateSPAD::EndOfEventAction(const G4Event *) {
    xray_global_time = -1;
    xray_creation_global_time = -1;
    if (!has_hitted)
        return;

    if (last_photon.empty()){
        last_photon[event_id] = photon_id;
        return;
    }
    auto last_photon_id = last_photon.rend()->second;
    if (last_photon_id != photon_id)
        last_photon[event_id] = photon_id;

    has_hitted = false;
}

void GateSPAD::PostUserTrackingAction(const GateVVolume *, const G4Track *) {

}

void GateSPAD::BeginOfRunAction(const G4Run *) {

}
void GateSPAD::EndOfRunAction(const G4Run *) {
}


void GateSPAD::BeginOfEventAction(const G4Event *) {

}


//get all photons from the acquisition and simulate all spad bevahior
void GateSPAD::RecordEndOfAcquisition(){
    auto avalanches = new Vector<Avalanche *>();
    for (uint i = 0; i < spacial_repartition_photon.size(); i++){
        auto spadMatrixX = i % matrix_spad_size + 1;
        auto spadMatrixY = i / matrix_spad_size + 1;
        //propertiesMappingType
        simulator->resetMatrix();
        if (local_dsas){
            auto iter = last_photon.begin();
            auto vec_phot = spacial_repartition_photon[i];
            uint j = 0;
            uint begin_index = 0;
            uint end_index = 0;
            while (j < vec_phot.size()){
                auto phot = vec_phot[j];
                auto phot_id = phot->getId();
                auto last_event_id = iter->first;
                auto last_photon_id = iter->second;
                if (phot_id <= last_photon_id){
                    j++;
                    if (j < vec_phot.size() )
                        continue;
                }
                else{
                    iter++;
                }
                end_index = j;

                if (end_index  == begin_index)
                    continue;

                Vector<Photon*> sub_photon;
                auto max_local_time = 0;
                for (auto p = begin_index; p < end_index; p++){
                    auto vec = vec_phot[p];
                    auto local_time = vec->getGlobalTime() - xray_emission_time[last_event_id];
                    vec->setGlobalTime(local_time);
                    max_local_time = max(local_time, max_local_time);
                    sub_photon.push_back(vec);
                }
                if (nVerboseLevel>1) G4cout << "n photons : " << end_index - begin_index  << Gateendl;
                simulator->resetMatrix();
                simulator->setSimulationTime(local_dsas_mesure_time);
                simulator->simulateAll(sub_photon, *avalanches);
                begin_index = j;
            }
        }
        else{
            simulator->simulateAll(spacial_repartition_photon[i], *avalanches);
        }
        if (avalanches->empty())
            continue;
        if (nVerboseLevel>1) G4cout << "avalanche size : " << avalanches->size() << " SPAD cord : " << spadMatrixX << " " << spadMatrixY << " " << Gateendl;
        exporter->exportSimulation(*avalanches,spadMatrixX,spadMatrixY);
        classic_exporter->exportSimulation(last_event_id, end_index - begin_index, *avalanches);
        avalanches->clear();
    }
    exporter->exportSimulation(&xray_emission_time);
    exporter->exportSimulation(spacial_repartition_photon);
    exporter->write_data();
    delete simulator;
    delete exporter;
    delete classic_exporter;
}

void GateSPAD::UserSteppingAction(const GateVVolume *, const G4Step *step) {
    auto lv = step->GetTrack()->GetNextVolume();
    if (xray_creation_global_time < 0){
        event_id++;
        auto v = step->GetTrack()->GetVolume()->GetName();
        if (v == "world_phys"){//particle enters the world
            xray_creation_global_time = step->GetPreStepPoint()->GetGlobalTime();
            G4cout << xray_creation_global_time << Gateendl;
    
            xray_emission_time.push_back(xray_creation_global_time);}
    }

    if (lv == nullptr)
        return;
    auto name = lv->GetName();
    if (xray_global_time < 0 and name == "crystal_phys"){
        xray_global_time = step->GetTrack()->GetGlobalTime();//get xray_global_time / position of the xray Primary Particle when it touches the crystal
        auto pos_xray = step->GetTrack()->GetPosition();
        auto energy = step->GetPreStepPoint()->GetKineticEnergy();
        auto wavelength = c_light * h_Planck / energy / nanometer;
        //save the index of the last registered photon in order to be able to track the gamma/xray source afterward
        exporter->exportSimulation(spacial_repartition_photon.size(), xray_global_time, pos_xray.x(), pos_xray.y(), wavelength);
        }
    if (name != "pixel_phys")
        return;

    auto energy = step->GetTotalEnergyDeposit();
    if (energy == 0){//no interaction with the detector
        return;
    }
    auto wavelength = c_light * h_Planck / energy / nanometer;

    auto stepPoint = step->GetPostStepPoint();
    G4ThreeVector polarization = stepPoint->GetMomentum(); //TODO : check normalized form
    auto time = stepPoint->GetGlobalTime();
    //auto position = stepPoint->GetPosition();
    auto id = lv->GetInstanceID();
    G4TouchableHandle theTouchable = stepPoint->GetTouchableHandle();
    G4ThreeVector worldPosition = stepPoint->GetPosition();
    G4ThreeVector position = theTouchable->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

    auto localPosition = Position2D((position.x())/um+detector_w/2, (position.y())/um+detector_h/2);
    auto polarizationSPAD = Position3D(polarization.x()/um, polarization.y()/um, polarization.z()/um);

    auto *p = new Photon(time, localPosition , polarizationSPAD,wavelength,++photon_id);

    spacial_repartition_photon[geometry_id_spad_id_map[id]].push_back(p);
    has_hitted = true;
    if (nVerboseLevel>1) G4cout << "spad hitted [ " << time << " , "<< id << " ] by photon " << photon_id << " " << localPosition.x << "," << localPosition.y << Gateendl;
    step->GetTrack()->SetTrackStatus(fStopAndKill); //the simulation of the optic photon will be done by DSAS
}

void GateSPAD::PreUserTrackingAction(const GateVVolume *, const G4Track *) {

}



GateSPAD::~GateSPAD() {
}
