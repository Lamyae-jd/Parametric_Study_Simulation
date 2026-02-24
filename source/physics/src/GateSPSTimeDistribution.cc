/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

#include <cmath>
#include <vector>
#include <fstream>

#include <G4Types.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>
#include <Randomize.hh>
#include <G4ParticleDefinition.hh>

#include "GateMessageManager.hh"
#include "GateMiscFunctions.hh"

#include "GateSPSTimeDistribution.hh"

using namespace std;


//-----------------------------------------------------------------------------
GateSPSTimeDistribution::GateSPSTimeDistribution()
{
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
G4double GateSPSTimeDistribution::GenerateOne()
{
  return GenerateFromUserSpectrum();;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// BuildSpectrum fonction read a file which contain in first line mode of spectrum (Spectrum line (1), Histogramm (2)
// or linear interpolated spectrum (3)) in first and in second start energy for the spectrum. Start energy it used only
// for histogram mode but you want give default value (0 for example), even if this value don't serve for mode (1) and (3)
// After the first line each line contain in first column energy and in second probability for this energy.
void GateSPSTimeDistribution::BuildUserSpectrum(G4String fileName)
{
  std::ifstream inputFile (fileName.data());
  G4int nline = 0;
  if(inputFile) {
    skipComment(inputFile);

    G4String line;
    G4double probaRead;
    G4double timeRead;

    G4int cursorPosition = inputFile.tellg();  // tellg() save file cursor position

    while(getline(inputFile, line)) nline++;  // count number of line of inputFile
    mDimSpectrum = nline - 1;

    // create two tables for energy and probability
    mTabTime.resize(mDimSpectrum);
    mTabProba.resize(mDimSpectrum);

    nline = 0;

    inputFile.clear();
    inputFile.seekg(cursorPosition, inputFile.beg);  // return to the 2nd line in the file

    while(nline < mDimSpectrum) {
      inputFile >> timeRead;
      inputFile >> probaRead;

      mTabTime[nline] = timeRead;
      mTabProba[nline] = probaRead;
      nline++;
    }

    inputFile.close();

    // Construct probability table
    mSumProba = 0;
    nline = 0;


    GateMessage("Time distribution of pulses", 2, "Reading UserSpectrum." << Gateendl);
    mTabSumProba.resize(mDimSpectrum);
    mSumProba = mTabProba[0] * (mTabTime[0]);
    mTabSumProba[0] = mSumProba;
    for(nline = 1; nline < mDimSpectrum; nline++) {
      mSumProba += (mTabTime[nline] - mTabTime[nline - 1]) * mTabProba[nline];
      mTabSumProba[nline] = mSumProba;
    }
    GateMessage("Time distribution of pulses", 2, "Reading UserSpectrum done. " << mDimSpectrum << " bins." << Gateendl);
  
  } else {
    std::string s = "The User Spectrum file '" + fileName + "' is not found.";
    G4Exception("GateSPSTimeDistribution::BuildUserSpectrum", "BuildUserSpectrum", FatalException, s.c_str());
  }
}

//-----------------------------------------------------------------------------
// Inverse transform sampling
G4double GateSPSTimeDistribution::GenerateFromUserSpectrum()
{
  G4double pTime = 0;
  G4double pStart = 0;

  // random cumulative probabability in ]0...1[
  G4double U = G4UniformRand();

  // identify corresponding interval of the tabulated cumulative distribution
  G4int i = 0;
  while( U >= (mTabSumProba[i] / mSumProba) ) i++;

 // histogram spectrum:
 // sample from uniform sub-distribution of the intervall
 if(i == 0) {
   pTime = G4RandFlat::shoot(pStart, mTabTime[0]);
 } else {
   pTime = G4RandFlat::shoot(mTabTime[i - 1], mTabTime[i]);
 }
 return pTime;
}
//-----------------------------------------------------------------------------
