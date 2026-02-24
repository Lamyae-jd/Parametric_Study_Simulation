/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/

/* ---------------------------------------------------------------------------- *
 *                                                                              *
 *  Class Description :                                                         *
 *                                                                              *
 *  To generate the energy of a primary vertex according to the defined         *
 *  distribution                                                                *
 *                                                                              *
 *   Revision 1.5 2014/08/1 Yann PERROT and Simon NICOLAS  LPC Clermont-ferrand *
 *   Solution for generating particles from Energy spectra (discrete spectrum,  *
 *   histogram and linear interpolation)                                        *
 *   Creation of two new methods:                                               *
 *     ConstructUserSpectrum() and GenerateFromUserSpectrum()                   *
 * -----------------------------------------------------------------------------*/

#ifndef GateSPSTimeDistribution_h
#define GateSPSTimeDistribution_h 1

#include <vector>

#include <G4Types.hh>
#include <G4String.hh>



class GateSPSTimeDistribution 
{
public:
  GateSPSTimeDistribution();

  // Shoot an energy in previously created probability tables
  G4double GenerateFromUserSpectrum();

  // Create probability tables
  void BuildUserSpectrum(G4String fileName);

  G4double GenerateOne();

private:

  std::vector<G4double> mTabProba;
  std::vector<G4double> mTabSumProba;
  std::vector<G4double> mTabTime;
  G4int mDimSpectrum;
  G4double mSumProba;
  
};

#endif  // GateSPSEneDistribution_h
