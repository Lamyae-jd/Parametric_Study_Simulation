/* FILE_INFO_START
 * Copyright (C) 2016 
 * 
 * ***********************************************************************
 * This file is part of SpadSimulator
 * 
 * SpadSimulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * SpadSimulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SpadSimulator.  If not, see <http://www.gnu.org/licenses/>.
 * ***********************************************************************
 * 
 * File        : root_simulation_exporter.cc
 * Author      : Audrey Corbeil Therrien <audrey.corbeil.therrien@usherbrooke.ca>
 * Author      : Valentin Libioulle <valentin.libioulle@usherbrooke.ca>
 * Created     : Wed Mar 16 16:01:05 2016 -0400 by valentin
 * 
 * FILE_INFO_END */


#include <TTree.h>
#include <TFile.h>
#include <TROOT.h>

#include "avalanche.hh"
#include "log.hh"
#include "root_custom_exporter.hh"
#include "root_include_lib.hh"

RootSimulationCustomExporter::RootSimulationCustomExporter(const std::string& filepath, const std::string& filter)
: SimulationExporter(filter)
, filePath(filepath)
{
    Clear();
    if (!vectorIncluded) {
        gROOT->ProcessLine("#include <vector>");
        vectorIncluded = true;
    }

    file = new TFile(filepath.c_str(), "RECREATE", "", 9);
    pxray_emission_time = &xray_emission_time;

    pXray_x = &xray_x;
    pXray_y = &xray_y;
    pXray_wavelength = &xray_wavelength;
    pXray_time = &xray_time;
    pfirstIndexPhot = &firstIndexPhot;

    tree_source_xray = new TTree("tree_source","name");
    tree_source_xray->Branch("GlobalTime", "std::vector<double>", &pxray_emission_time);

    tree_xray = new TTree("tree_pre_scintillator","name");
    tree_xray->Branch("posX", "std::vector<double>", &pXray_x);
    tree_xray->Branch("posY", "std::vector<double>", &pXray_y);
    tree_xray->Branch("GlobalTime", "std::vector<double>", &pXray_time);
    tree_xray->Branch("Wavelength", "std::vector<double>", &pXray_wavelength);
    tree_xray->Branch("FirstIndexPhotId", "std::vector<uint>", &pfirstIndexPhot);


    pGlobalTime = &globalTime;
    pTriggerType = &triggerType;
    pSpadX = &spadX;
    pSpadY = &spadY;
    pPhotonid = &photonid;

    tree = new TTree("tree_dsas", "name");
    tree->Branch("SpadMatrixX", &SpadMatrixX,"SpadMatrixX/s");
    tree->Branch("SpadMatrixY", &SpadMatrixY,"SpadMatrixY/s");
    tree->Branch("GlobalTime", "std::vector<float>", &pGlobalTime);
    tree->Branch("TriggerType", "std::vector<int>", &pTriggerType);
    tree->Branch("SpadX", "std::vector<ushort>", &pSpadX);
    tree->Branch("SpadY", "std::vector<ushort>", &pSpadY);
    tree->Branch("PhotonId", "std::vector<uint>", &pPhotonid);

    pPhotonid2 = &photonid2;
    pPosX = &posX;
    pPosY = &posY;
    pGlobalTime2 = &globalTime2;
    pWavelength = &wavelength;


    tree_phot = new TTree("tree_detector","name");
    tree_phot->Branch("SpadMatrixX", &SpadMatrixX,"SpadMatrixX/s");
    tree_phot->Branch("SpadMatrixY", &SpadMatrixY,"SpadMatrixY/s");
    tree_phot->Branch("PhotonId", "std::vector<uint>", &pPhotonid2);
    tree_phot->Branch("posX", "std::vector<double>", &pPosX);
    tree_phot->Branch("posY", "std::vector<double>", &pPosY);
    tree_phot->Branch("GlobalTime", "std::vector<double>", &pGlobalTime2);
    tree_phot->Branch("Wavelength", "std::vector<double>", &pWavelength);



}


void RootSimulationCustomExporter::Clear()
{
    xray_time.clear();
    xray_wavelength.clear();
    xray_x.clear();
    xray_y.clear();
    eventId = 0;
    photonid.clear();
    triggerType.clear();
    globalTime.clear();
    spadX.clear();
    spadY.clear();

    posX.clear();
    posY.clear();
    photonid2.clear();
    globalTime2.clear();
    wavelength.clear();
}


void RootSimulationCustomExporter::write_data(){
    firstIndexPhot.push_back(photonid2.size());
    tree_xray->Fill();
    tree_source_xray->Fill();
    //tree->Write();
    //tree_phot->Write();
    //tree_xray->Write();
    file->Write();
    file->Close();
    Clear();
}
void RootSimulationCustomExporter::exportSimulation(std::vector<double> *global_time_xray_emission) {
    pxray_emission_time =  global_time_xray_emission;
}

void RootSimulationCustomExporter::exportSimulation(const Vector<Vector<Photon*>> &timed_photon) {

    for (uint j = 0; j < timed_photon.size(); j++){
        auto spadMatrixX = j % 8 + 1;
        auto spadMatrixY = j / 8 + 1;
        SpadMatrixX = spadMatrixX;
        SpadMatrixY = spadMatrixY;

        auto phot_array = timed_photon[j];
        auto length = phot_array.size();
        for (uint i = 0; i < length; i++) {
            auto phot = phot_array[i];
            auto pos = phot->getPenetrationPoint();
            auto time = phot->getGlobalTime();
            posX.push_back(pos.x);
            posY.push_back(pos.y);
            photonid2.push_back(phot->getId());
            globalTime2.push_back(time);
            wavelength.push_back(phot->getWavelength());
        }
        tree_phot->Fill();
        posX.clear();
        posY.clear();
        photonid2.clear();
        globalTime2.clear();
        wavelength.clear();
    }
    tree_phot->Print();
}

void RootSimulationCustomExporter::exportSimulation(uint firstIndexPhoto, double global_time, double x, double y, double localwavelength) {

    firstIndexPhot.push_back(firstIndexPhoto);
    xray_x.push_back(x);
    xray_y.push_back(y);
    xray_time.push_back(global_time);
    xray_wavelength.push_back(localwavelength);
}


void RootSimulationCustomExporter::exportSimulation(const Vector< Avalanche* >& avalanches, uint spadMatrixX1,uint spadMatrixY1)
{
    Vector<Avalanche*>::const_iterator it;
    int exportedAvalancheCount = 0;
    int triggeredAvalanche = 0;
    SpadMatrixX = spadMatrixX1;
    SpadMatrixY = spadMatrixY1;
    for (it = avalanches.begin(); it != avalanches.end(); ++it) {
        Avalanche* avalanche = *it;
        if (avalancheIsExported(*avalanche)) {
            globalTime.push_back(avalanche->getGlobalTime());
            triggerType.push_back(avalanche->getTriggerType());
            spadX.push_back(avalanche->getSpadCoordinate().x);
            spadY.push_back(avalanche->getSpadCoordinate().y);
            if (avalanche->getTriggerType() == Particle::PHOTON)
                photonid.push_back(avalanche->getPhotonId());
            else{
                photonid.push_back(0);
            }
            ++exportedAvalancheCount;
        }
        if (!avalanche->isMasked()) {
            ++triggeredAvalanche;
        }
    }



    
    LOG_STREAM(2) << "Data Exported :" << std::endl; 
    LOG_STREAM(2) << "  Event ID :\t   " << eventId << std::endl;
    LOG_STREAM(2) << "  SPAD Triggered Count : " << triggeredAvalanche << std::endl;
    LOG_STREAM(2) << "  Avalanche Count :\t   " << exportedAvalancheCount << std::endl;
    
    tree->Fill();
}


RootSimulationCustomExporter::~RootSimulationCustomExporter(){

}
