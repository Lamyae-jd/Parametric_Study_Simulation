/*----------------------
   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/


#include "GateSourceVoxellizedPulsedMessenger.hh"
#include "GateSourceVoxellizedPulsed.hh"

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"

//-----------------------------------------------------------------------------
GateSourceVoxellizedPulsedMessenger::GateSourceVoxellizedPulsedMessenger(GateSourceVoxellizedPulsed* source)
  : GateMessenger(G4String("source/") + source->GetName(), false),
    m_source(source)
{
  G4String cmdName;

  cmdName = GetDirectoryName()+"reader/insert";
  ReaderInsertCmd = new GateUIcmdWithAVector<G4String>(cmdName,this);
  ReaderInsertCmd->SetGuidance("Insert a reader");
  ReaderInsertCmd->SetGuidance("1. Reader type");

  cmdName = GetDirectoryName()+"reader/remove";
  ReaderRemoveCmd = new G4UIcmdWithoutParameter(cmdName,this);
  ReaderRemoveCmd->SetGuidance("Remove the reader");

  cmdName = GetDirectoryName()+"setPosition";
  PositionCmd = new G4UIcmdWith3VectorAndUnit(cmdName,this);
  PositionCmd->SetGuidance("Set the source global position");
  PositionCmd->SetGuidance("1. Position xyz and unit");

  cmdName = GetDirectoryName()+"TranslateTheSourceAtThisIsoCenter";
  translateIsoCenterCmd = new G4UIcmdWith3VectorAndUnit(cmdName,this);
  translateIsoCenterCmd->SetGuidance("Set the source position so that the given position is at world 0,0,0");
  translateIsoCenterCmd->SetUnitCategory("Length");

 
  cmdName = GetDirectoryName()+"setPulseJitter";
  pJitterCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);

  cmdName = GetDirectoryName()+"setPulseDelay";
  pDelayCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);
  
  cmdName = GetDirectoryName()+"setFrequency";
  pFrequencyCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);
  
  cmdName = GetDirectoryName()+"setPhotonAvg";
  pAvgPhotonCmd = new G4UIcmdWithADouble(cmdName,this);
  cmdName = GetDirectoryName()+"setPhotonSigma";
  pSigmaPhotonCmd = new G4UIcmdWithADouble(cmdName,this);
  
  
  cmdName = GetDirectoryName()+"setTimeHist";
  pTimeHistCmd = new G4UIcmdWithAString(cmdName,this);

  
  cmdName = GetDirectoryName()+"setPulseEnd";
  pEndCmd = new G4UIcmdWithADoubleAndUnit(cmdName,this);

}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
GateSourceVoxellizedPulsedMessenger::~GateSourceVoxellizedPulsedMessenger()
{
   delete PositionCmd;
   delete ReaderInsertCmd;
   delete ReaderRemoveCmd;
   delete translateIsoCenterCmd;
    delete pJitterCmd;
    delete pDelayCmd;
    delete pFrequencyCmd;
    delete pAvgPhotonCmd;
    delete pSigmaPhotonCmd;
    delete pTimeHistCmd;
    delete pEndCmd;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GateSourceVoxellizedPulsedMessenger::SetNewValue(G4UIcommand* command,G4String newValue)
{
  if (command == ReaderInsertCmd)  m_source->ReaderInsert(ReaderInsertCmd->GetNewVectorValue(newValue)[0]);
  if (command == ReaderRemoveCmd)  m_source->ReaderRemove();
  if (command == PositionCmd) m_source->SetPosition(PositionCmd->GetNew3VectorValue(newValue));
  if (command == translateIsoCenterCmd) m_source->SetIsoCenterPosition(translateIsoCenterCmd->GetNew3VectorValue(newValue));
  if (command == pJitterCmd) {m_source->SetJitter(pJitterCmd->GetNewDoubleValue(newValue)); }
  if (command == pDelayCmd) {m_source->SetDelay(pDelayCmd->GetNewDoubleValue(newValue)); }
  if (command == pFrequencyCmd) {m_source->SetFrequency(pFrequencyCmd->GetNewDoubleValue(newValue)); }
  if (command == pAvgPhotonCmd) {m_source->SetAvgPhoton(pAvgPhotonCmd->GetNewDoubleValue(newValue)); }
  if (command == pSigmaPhotonCmd) {m_source->SetSigmaPhoton(pSigmaPhotonCmd->GetNewDoubleValue(newValue)); }
  if (command == pTimeHistCmd) {m_source->SetTimeFile(newValue); }
  if (command == pEndCmd) {m_source->SetEndTime(pEndCmd->GetNewDoubleValue(newValue)); }
  GateMessenger::SetNewValue(command, newValue);
}
//-----------------------------------------------------------------------------
