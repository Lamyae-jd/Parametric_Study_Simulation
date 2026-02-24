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
 * File        : root_simulation_exporter.hh
 * Author      : Audrey Corbeil Therrien <audrey.corbeil.therrien@usherbrooke.ca>
 * Author      : Valentin Libioulle <valentin.libioulle@usherbrooke.ca>
 * Created     : Wed Mar 16 16:01:05 2016 -0400 by valentin
 * 
 * FILE_INFO_END */


#ifndef ROOT_EVENT_CUSTOM_EXPORTER_ROOT_H
#define ROOT_EVENT_CUSTOM_EXPORTER_ROOT_H

#include "simulation_exporter.hh"
#include <vector>

class TFile;
class TTree;

/**
 * @brief Derived SimulationExporter that write Root file (.root).
 */
class RootSimulationCustomExporter : public SimulationExporter {
private:
    RootSimulationCustomExporter(const RootSimulationCustomExporter&);
    RootSimulationCustomExporter& operator=(const RootSimulationCustomExporter&);
    
    std::string filePath;
    
    TFile* file;
    std::vector<double> xray_emission_time;
    std::vector<double> *pxray_emission_time;

    std::vector<double> xray_x, xray_y, xray_time, xray_wavelength;
    std::vector<double> *pXray_x, *pXray_y, *pXray_time, *pXray_wavelength;
    std::vector<uint> firstIndexPhot, *pfirstIndexPhot;
    ushort SpadMatrixY, SpadMatrixX;

    std::vector<float> globalTime,  *pGlobalTime;
    std::vector<ushort> spadX, spadY, *pSpadX, *pSpadY;
    std::vector<int> triggerType, *pTriggerType;
    std::vector<uint> photonid, *pPhotonid;


    std::vector<double> globalTime2, *pGlobalTime2, wavelength, *pWavelength;
    std::vector<double> posX, posY, *pPosX, *pPosY;
    std::vector<uint> photonid2, *pPhotonid2;
    int eventId;
    
    void Clear();
    
public:
    TTree* tree, *tree_phot, *tree_xray, *tree_source_xray;
    /**
     * @brief Constructor.
     * 
     * @param filePath The path to the file to create. (ex: /home/bob/Simulation.root)
     * @param filter The filter that indicate which avalanches are exported.
     */
    RootSimulationCustomExporter(const std::string& filePath, const std::string& filter);
    
    /**
     * @brief Destructor.
     */
    virtual ~RootSimulationCustomExporter() ;
    
    virtual void exportSimulation(const Vector<Avalanche*>& avalanches, uint spadMatrixX,uint spadMatrixY);

    virtual void exportSimulation(const int& ,const int& , const Vector<Avalanche*>& ){};

    void exportSimulation(const Vector<Vector<Photon *>> &timed_photon);

    void exportSimulation(uint firstIndexPhoto, double global_time, double x, double y, double localwavelength);

    void write_data();

    void exportSimulation(const Vector<double> &timed_photon);

    void exportSimulation(std::vector<double> *global_time_xray_emission);
};


#endif
