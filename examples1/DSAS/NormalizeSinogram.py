import sys
import os
import SimpleITK as sitk
import argparse
import math
import numpy as np

from pathlib import Path
package_directory = (Path(__file__).parent / "..").resolve()
sys.path.append(str(package_directory))
import Utility as ut




def main(outputDir, params):
    if(not("InputDir" in params)):
        ut.printError("Unspecified input directory, cannot normalize image data")
        return  
    if(not("DecayFile" in params)):
        ut.printError("Unspecified decay file, cannot normalize image data")
        return
    if(not os.path.isabs(params["DecayFile"])):
        params["DecayFile"] = outputDir + "/" + params["DecayFile"]
    # if(not os.path.isabs(params["InputDir"])):
    #     params["InputDir"] = outputDir + "/" + params["InputDir"]
    InputDir = params["InputDir"]

    toNormalize = []
    for file in os.listdir(InputDir):
        if file.endswith(".mha"):
            toNormalize.append(file)

    print("File to Normalize")
    print(toNormalize)
    for file in toNormalize:
        filename = file.replace(".mha", "")
        inputFileName = InputDir + file
        outputFileName = outputDir + "/" + filename + "Normalized.mha"
        normalizeSinogram(params["DecayFile"], inputFileName, outputFileName,params)

def fuse(outputDir, params, dirList):
    if(not("InputDir" in params)):
        ut.printError("Unspecified input directory, cannot normalize image data")
        return
    if(not("DecayFile" in params)):
        ut.printError("Unspecified decay file, cannot normalize image data_")
        return
    if(not os.path.isabs(params["DecayFile"])):
        params["DecayFile"] = outputDir + "/" + params["DecayFile"]
    if(not os.path.isabs(params["InputDir"])):
        params["InputDir"] = outputDir + "/" + params["InputDir"]
    InputDir = params["InputDir"]

    toNormalize = []
    for file in os.listdir(InputDir):
        if file.endswith(".mha"):
            toNormalize.append(file)

    print("File to Normalize")
    print(toNormalize)
    for file in toNormalize:
        inputFileName = InputDir + file
        outputFileName = outputDir + "/" + file.replace(".mha", "") + "Normalized.mha"
        normalizeSinogram(params["DecayFile"], inputFileName, outputFileName,params)
    return
   
def normalizeSinogram(decayFile, inputFile, outputFile, params):
    i0 = 1
    nbOfPixelX = 30
    nbOfPixelY = 30
    nbOfProjections = 1
    nbOfSlices = 1
    sourceAngle = 8.9
    SDD = 1000
    pixelSize = 1
    pixelDepth = 2.5
    totalPhotons = 0
    energyInit = 1
    if("PanelHeight" in params and "PixelSize" in params):
        nbOfPixelY = int(float(params["PanelHeight"])/float(params["PixelSize"]))
    if("PanelWidth" in params and "PixelSize" in params):
        nbOfPixelX = int(float(params["PanelWidth"])/float(params["PixelSize"]))
    if("PixelSize" in params):
        pixelSize = float(params["PixelSize"])
    if("PixelDepth" in params):
        pixelDepth = int(params["PixelDepth"])
    if("Projections" in params):
        nbOfProjections = int(params["Projections"])
    if ("Slices" in params):
        nbOfSlices = int(params["Slices"])
    if("SourceAngle" in params):
        sourceAngle = int(params["SourceAngle"])
    if("SDD" in params):
        SDD = int(params["SDD"])
    if("EnergySource" in params and "Energy" in inputFile):
        energyInit = int(params["EnergySource"])*0.001
    if(os.path.isfile(decayFile)):
        
        fileDecay = open(decayFile, "r", encoding='utf-8') 
        for line in fileDecay:
            PhotonPerSlice = int(line.strip())
            totalPhotons = totalPhotons + PhotonPerSlice 
            
        # for i in range(0,nbOfProjections * nbOfSlices):
        #     PhotonPerSlice = int(fileDecay.strip())
        #     totalPhotons = totalPhotons + PhotonPerSlice
        fileDecay.close()
    else:
        ut.printError("Unspecified decay file, cannot normalize image data_")
        return

    sinReader = sitk.ImageFileReader()
    sinReader.SetFileName(str(inputFile))
    source = sinReader.Execute()
    PhotonPerProjection = totalPhotons/(nbOfProjections * nbOfSlices)
    if params["BeamType"] == "cone":
        area = (math.tan(math.radians(sourceAngle))*SDD)**2*math.pi
    elif params["BeamType"] == "fan":
        area = 4*(math.tan(math.radians(sourceAngle))*SDD)*(math.tan(math.radians(0.04))*SDD) #TO CORRECT FOR FAN ANGLE
    elif params["BeamType"] == "parallel":
        area = 4*params["halfx"]*params["halfy"]
    else:
        ut.printError("Unspecified beam shape, cannot normalize image data")
        return

    if inputFile.count("Summed") != 0:
        nbOfPixelY = 1
    for k in ut.progressbar(range(0,nbOfProjections),prefix="Normalizing voxel data"):
        i0 = (PhotonPerProjection/area) * pixelSize * pixelSize
        absorption = 0.30689816*math.log(pixelDepth)+0.563803707
        i0 = i0*absorption
        i0 = i0*energyInit #energy is equal to 1 in photoncounting mode
        for i in range(0,nbOfPixelX):
            for j in range(0,nbOfPixelY):
                if(source.GetPixel(i,j,k)>0):
                    source.SetPixel(i,j,k,math.log(i0/(source.GetPixel(i,j,k)))) #Method from physics in nuclear medicine
                else:
                    source.SetPixel(i,j,k,0)

    sinWriter = sitk.ImageFileWriter()
    sinWriter.SetFileName(str(outputFile))
    sinWriter.Execute(source);


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="generate macro files for Gate and run the simulation")
    parser.add_argument("Output", help="output directory")
    parser.add_argument("-p", "--params", action='append', type=lambda kv: kv.split("=", 1), dest='params')
    args = parser.parse_args()   
    params = dict(args.params) 
    main(args.Output, params)
