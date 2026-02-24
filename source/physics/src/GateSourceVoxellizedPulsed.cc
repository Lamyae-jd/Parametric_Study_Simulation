/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


#include "GateSourceVoxellizedPulsed.hh"
#include "GateSourceVoxellizedPulsedMessenger.hh"
#include "GateVSourceVoxelReader.hh"
#include "GateSourceVoxelTestReader.hh"
#include "GateSourceVoxelImageReader.hh"
#include "GateSourceVoxelInterfileReader.hh"

//-------------------------------------------------------------------------------------------------
GateSourceVoxellizedPulsed::GateSourceVoxellizedPulsed(G4String name)
  : GateVSource(name)
  , m_sourcePosition(G4ThreeVector())
  , m_sourceRotation(G4RotationMatrix())
  , m_voxelReader(0)
{
  CLHEP::HepRandomEngine *engine = GateRandomEngine::GetInstance()->GetRandomEngine();
  mLastPhoton = 0;
  mPulseStart = 0;
  mPeriod = 1;
  mJitter = 0;
  mDelay = 0;
  mPhotonAvg = 1;
  mPhotonSigma = 0;
  mEndTime = 1;
  mPhotonList = new std::vector<double>();
  mPhotonCountDistribution = new RandGauss(engine);
  mPulseJitterDistribution = new RandGauss(engine);
  mTimeDistribution = new GateSPSTimeDistribution();
  m_sourceVoxellizedPulsedMessenger = new GateSourceVoxellizedPulsedMessenger(this);
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
GateSourceVoxellizedPulsed::~GateSourceVoxellizedPulsed()
{
  if (nVerboseLevel > 0)
    G4cout << "GateSourceVoxellized::~GateSourceVoxellized \n";
  delete m_sourceVoxellizedPulsedMessenger;
  if (m_voxelReader) delete m_voxelReader;
  delete mPhotonList;
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
G4double GateSourceVoxellizedPulsed::GetNextTime(G4double timeNow)
{
    double computedTime = 0.0;
    double jitterStart = 0.0;
    double numberOfPhoton = 1.0;
    //G4cout << "*****Generating time ****\n" << std::fixed << std::setprecision(9) << "timeStart:" <<  timeStart << "\n";
    
    while(mPhotonList->size() == 0)
    {
         mPulseStart += mPeriod;
         //computedTime = (mPulseStart - timeStart);
         if(mPulseStart > mEndTime)
         {
             return mEndTime;
         }
         jitterStart = mPulseStart + ((mPulseJitterDistribution->fire(mDelay, mJitter)));
         numberOfPhoton = mPhotonCountDistribution->fire(mPhotonAvg,mPhotonSigma);
         for(double i = numberOfPhoton; i > 0.5; i--) 
         {
            mPhotonList->push_back(jitterStart+(mTimeDistribution->GenerateOne()));
         }
         std::sort(mPhotonList->begin(), mPhotonList->end());
         std::reverse(mPhotonList->begin(), mPhotonList->end());
    }   
    computedTime = mPhotonList->back();
    //G4cout << "computedTime:" <<  computedTime << "\n";
    mPhotonList->pop_back();
    return G4double(computedTime-timeNow);
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
void GateSourceVoxellizedPulsed::Dump(G4int level)
{
  G4cout << "Source ---------------> " << m_name << Gateendl
	 << "  ID                  : " << m_sourceID << Gateendl
	 << "  type                : " << m_type << Gateendl
	 << "  startTime (s)       : " << m_startTime/s << Gateendl
	 << "  time (s)            : " << m_time/s << Gateendl
	 << "  forcedUnstable      : " << m_forcedUnstableFlag << Gateendl;
  if ( m_forcedUnstableFlag )
    G4cout << "  forcedLifetime (s)  : " << m_forcedLifeTime/s << Gateendl;
  G4cout << "  verboseLevel        : " << nVerboseLevel << Gateendl
 	 << "----------------------- \n";

  if (!m_voxelReader) {
    G4cout << "GateSourceVoxellized::Dump: voxel reader not defined\n";
  } else {
    if (level > 0)
      m_voxelReader->Dump(1);
  }

  if (level > 0)
    GateVSource::Dump(1);

}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
G4int GateSourceVoxellizedPulsed::GeneratePrimaries(G4Event* event)
{
  if (!m_voxelReader) {
    G4cout << "GateSourceVoxellizedPulsed::GeneratePrimaries: insert a voxel reader first\n";
    return 0;
  }
  // ask to the voxel reader to provide the active voxel for this event
  G4int nextSource = m_voxelReader->GetNextSource();
  G4ThreeVector firstSource = m_voxelReader->GetVoxelIndices(nextSource);
  // move the centre to the chosen voxel:
  // to the relative position and then to the global absolute position taking into account
  // the rotation of the voxel matrix both for the position and for the orientation of the active voxel

  G4ThreeVector voxelSize = m_voxelReader->GetVoxelSize();
  // offset of the centre of the selected voxel wrt the matrix corner (as the (0,0,0) voxel is in the corner)
  G4ThreeVector relativeVoxelOffset = G4ThreeVector( voxelSize.x()/2. + voxelSize.x() * firstSource[0],
						     voxelSize.y()/2. + voxelSize.y() * firstSource[1],
						     voxelSize.z()/2. + voxelSize.z() * firstSource[2]);

  // m_sourcePosition and m_sourceRotation are NOT the ones in GPS, on the contrary they are used to set the
  // GPS position and "position rotation" (for the moment not the "direction rotation")
  G4ThreeVector centre = m_sourcePosition + m_sourceRotation(relativeVoxelOffset);

  // rotation of the Para shape according to the rotation of the voxel matrix
  GetPosDist()->SetPosRot1(m_sourceRotation(G4ThreeVector(1.,0.,0.))); // x'
  GetPosDist()->SetPosRot2(m_sourceRotation(G4ThreeVector(0.,1.,0.))); // y'

  if (nVerboseLevel > 1)
    G4cout << "[GateSourceVoxellizedPulsed::GeneratePrimaries] Centre: " << G4BestUnit(centre,"Length") << Gateendl;


  GetPosDist()->SetCentreCoords(centre);
  GetPosDist()->SetPosDisType("Volume");
  GetPosDist()->SetPosDisShape("Para");
  GetPosDist()->SetHalfX(voxelSize.x()/2.);
  GetPosDist()->SetHalfY(voxelSize.y()/2.);
  GetPosDist()->SetHalfZ(voxelSize.z()/2.);

  // shoot the primary
  G4int numVertices = GateVSource::GeneratePrimaries(event);

  return numVertices;
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
void GateSourceVoxellizedPulsed::ReaderInsert(G4String readerType)
{
  if (m_voxelReader) {
    G4cout << "GateSourceVoxellizedPulsed::ReaderInsert: voxel reader already defined\n";
  } else {
    if (readerType == G4String("test")) {
      m_voxelReader = new GateSourceVoxelTestReader(this);
    } else if (readerType == G4String("image")) {
      m_voxelReader = new GateSourceVoxelImageReader(this);
    } else if (readerType == G4String("interfile")) {
      m_voxelReader = new GateSourceVoxelInterfileReader(this);
    }
    else {
      G4cout << "GateSourceVoxellizedPulsed::ReaderInsert: unknown reader type\n";
    }
  }
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
void GateSourceVoxellizedPulsed::ReaderRemove()
{
  if (m_voxelReader) {
    delete m_voxelReader;
    m_voxelReader = 0;
  } else {
    G4cout << "GateSourceVoxellizedPulsed::ReaderRemove: voxel reader not defined\n";
  }
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
void GateSourceVoxellizedPulsed::Update(G4double time)
{
  // insert here any specific update needed for the voxel matrix source, otherwise for the standard updates
  // the GateVSource::Update() is called
  if (false) {

    // this will be applied only in the case the source is attached to a volume, thus it has to be aligned to
    // the actual volume position

    m_sourceRotation = G4RotationMatrix(); // for the moment set to unity

    // position of the volume center. This is obtained from the geometry.
    G4ThreeVector volumePosition       = G4ThreeVector(); // for the moment set to zero

    // offset of the matrix corner wrt the matrix centre. This is obtained from the geometry, it's given by
    // the half dimensions of the geometry voxel matrix. It is not a source property, as in the source there
    // is no concept of overall dimensions, like nx ny nz, just a set of dispersed voxels)
    // WRT the value given by the box dimensions, the offset has to be rotated according to the box orientation.
    G4ThreeVector relativeCornerOffset = m_sourceRotation(G4ThreeVector()); // for the moment set to zero

    // the source position for the case of the voxels is always the "corner" of the box
    m_sourcePosition = volumePosition - relativeCornerOffset; // attention to the "-" sign...

  } else {
    // the position is set through the source command
  }

  GateVSource::Update(time);
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------

void GateSourceVoxellizedPulsed::SetIsoCenterPosition(G4ThreeVector p)
{
  m_sourcePosition = m_voxelReader->ComputeSourcePositionFromIsoCenter(p);
}
//-------------------------------------------------------------------------------------------------
