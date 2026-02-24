//
// Created by loug2904 on 06/07/22.
//

#ifndef GATE_UTILS_H
#define GATE_UTILS_H

#include "isimulator.hh"
#include "ConfigFile.h"

ISimulator* getSimulatorFromConfig(const ConfigFile& config);

#endif //GATE_UTILS_H
