# Script de generation de sinogramme pour reconstruction FBP
# Par Julien Rossignol

import sys
import shutil
import os

from numpy.core.records import fromfile
from ROOT import TFile
from ROOT import gROOT, gDirectory
import math
import SimpleITK as sitk
import Utility as ut
import argparse
import numpy as np
from pathlib import Path
from filelock import Timeout, FileLock


def main(outputDir, params, nbOfProcs = 1):
    print(outputDir)
    outputDir = Path(outputDir)

    nbOfProjections = 1
    nbOfSlices = 1
    nbOfPixelX = 30
    nbOfPixelY = 30
    pixelSize = 1
    energySinogram = False
    estimateScatter = False
    ratio = False
    t0 = 0
    timeSinogram = False
    totalSinogram = True
    if(not("InputDir" in params)):
        ut.printError("Unspecified input file, cannot generate sinogram image data")
        return  
    if("Projections" in params):
        nbOfProjections = int(params["Projections"])
    if("Slices" in params):
        nbOfSlices = int(params["Slices"])
    if("PanelHeight" in params and "PixelSize" in params):
        nbOfPixelY = int(float(params["PanelHeight"])/float(params["PixelSize"]))
    if("PanelWidth" in params and "PixelSize" in params):
        nbOfPixelX = int(float(params["PanelWidth"])/float(params["PixelSize"]))*nbOfSlices
    if("Energy" in params):
        energySinogram = True
    if("EstimateScatter" in params):
        estimateScatter = params["EstimateScatter"]
    if ("ratio" in params):
        ratio = params["ratio"]
    if ("t0" in params):
        t0 = params["t0"]
    if ("timeSinogram" in params):
        timeSinogram = params["timeSinogram"]
    if ("totalSinogram" in params):
        totalSinogram = params["totalSinogram"]
    if(not("EstimatePixel" in params)):
        params["EstimatePixel"] = False
    if("PixelSize" in params):
        pixelSize = params["PixelSize"]
    

    
    #if(not os.path.isabs(params["InputDir"])):
     #   params["InputDir"] = outputDir / params["InputDir"]
    InputDir = Path(params["InputDir"])

    print("Lamyae:", InputDir)

   #Copying files without modifications
    shutil.copy2(InputDir / "decayRun.dat", outputDir)

    #SinoList of created sinogramme
    sinoList = []

    #Creation d'une image vide pour la reconstruction
    sizeOutput = [nbOfPixelX, nbOfPixelY, nbOfProjections]
    source = sitk.Image(sizeOutput, sitk.sitkFloat64)
    spacing = [pixelSize, pixelSize, 1]
    origin = [(nbOfPixelX-1)*pixelSize*-0.5, (nbOfPixelY-1)*pixelSize*-0.5, 0]
    source.SetOrigin(origin)
    source.SetSpacing(spacing)

    #creation d'une image vide pour le signogramme d'integration d'energie
    if(energySinogram):
        sourceEnergy = sitk.Image(sizeOutput, sitk.sitkFloat64)
        sourceEnergy.SetOrigin(origin)
        sourceEnergy.SetSpacing(spacing)


    #creation de deux images vides pour l'estimation de la diffusion
    if(estimateScatter):
        scatterSinogram = sitk.Image(sizeOutput, sitk.sitkFloat64)
        scatterSinogram.SetOrigin(origin)
        scatterSinogram.SetSpacing(spacing)
        primarySinogram = sitk.Image(sizeOutput, sitk.sitkFloat64)
        primarySinogram.SetOrigin(origin)
        primarySinogram.SetSpacing(spacing)
        if ratio:
            ratioSinogram = sitk.Image(sizeOutput, sitk.sitkFloat64)
            ratioSinogram.SetOrigin(origin)
            ratioSinogram.SetSpacing(spacing)


    fileRoot = TFile(str(InputDir / "outputCrystal.root"))  
    dataTree = fileRoot.Get("Singles")

    #Generate sinogram with filtered data
    for x in ut.progressbar(range(0, dataTree.GetEntries()), prefix="Creating 3D sinogram (filtered in)..."):
        dataTree.GetEntry(x)
        if(params["EstimatePixel"]):
            pixel = getPixelFromGlobalData(nbOfProjections, pixelSize ,nbOfSlices, nbOfPixelX, nbOfPixelY, dataTree.globalPosX, dataTree.globalPosY, dataTree.runID )
        else:
            pixel = getPixelFromData(nbOfProjections, nbOfSlices, nbOfPixelX, nbOfPixelY, dataTree.pixelID, dataTree.runID )
        source.SetPixel(pixel, source.GetPixel(pixel)+ 1)
        if(energySinogram):
            sourceEnergy.SetPixel(pixel , sourceEnergy.GetPixel(pixel) + dataTree.energy )
        if(estimateScatter):
            if(dataTree.comptonPhantom > 0 or dataTree.RayleighPhantom > 0):
                scatterSinogram.SetPixel(pixel , scatterSinogram.GetPixel(pixel) + 1 )
            else:    
                primarySinogram.SetPixel(pixel , primarySinogram.GetPixel(pixel) + 1 )

    if estimateScatter and ratio:
        for k in range(0, nbOfProjections):
            for i in range(0, nbOfPixelX):
                for j in range(0, nbOfPixelY):
                    if primarySinogram.GetPixel(i, j, k) == 0:
                        ratioSinogram.SetPixel(i, j, k, 0)
                    else:
                        ratioSinogram.SetPixel(i, j, k, scatterSinogram.GetPixel(i, j, k)/primarySinogram.GetPixel(i, j, k))

 

    sinWriter = sitk.ImageFileWriter()
    if (totalSinogram):
        sinWriter.SetFileName(str(outputDir / "TotalSinogram.mha"))
        sinWriter.Execute(source)
        sinoList.append(outputDir / "TotalSinogram.mha")
    if(energySinogram):
        sinWriter.SetFileName(str(outputDir / "TotalEnergySinogram.mha"))
        sinWriter.Execute(sourceEnergy)
        sinoList.append(outputDir / "TotalEnergySinogram.mha")
    if(estimateScatter):
        sinWriter.SetFileName(str(outputDir / "TotalScatterSinogram.mha"))
        sinWriter.Execute(scatterSinogram)
        sinoList.append(outputDir / "TotalScatterSinogram.mha")

def fuse(outputDir, params, dirList):
    outputDir = Path(outputDir)
    InputDir = Path(params["InputDir"])

    nbOfProjections = 1
    if("Projections" in params):
        nbOfProjections = int(params["Projections"])

    emittedPhotons = np.zeros(nbOfProjections, dtype=np.int)

    AddingtoFuse = {}
    for directory in dirList:
        directory = Path(directory)
        if(not(directory.exists())):
            continue
        
        for file in directory.iterdir():
            if file.suffix == ".mha":
                if file.name not in AddingtoFuse:
                    AddingtoFuse[file.name] = [file]
                else:
                    AddingtoFuse[file.name].append(file)
        decayPath = directory / "decayRun.dat"
        if decayPath.is_file():
            emittedPhotons = np.sum([emittedPhotons, np.fromfile(decayPath,dtype=int,sep='\n')],axis=0)


    for fileName, ListDirectory in AddingtoFuse.items():
        addSinogram(ListDirectory, str(outputDir / fileName))

    decayOutPath = outputDir / "decayRun.dat"
    if(not(decayOutPath.is_file())):
        emittedPhotons.tofile(outputDir / "decayRun.dat",sep='\n')


def getPixelFromData(nbOfProjections, nbOfSlices, nbOfPixelX, nbOfPixelY, pixelID, runID): #For now only slice along x axis is allowed
    detectorWidth = nbOfPixelX / nbOfSlices
    projection = runID % nbOfProjections
    sliceOffset = int(runID / nbOfProjections)
    sliceOffsetX = int(detectorWidth*(nbOfSlices - 1) - detectorWidth*sliceOffset)
    pixelX = (pixelID / nbOfPixelY) + sliceOffsetX
    pixelY = (pixelID % nbOfPixelY)
    return [int(pixelX), int(pixelY), int(projection)]

def getPixelFromGlobalData(nbOfProjections, pixelsize, nbOfSlices, nbOfPixelX, nbOfPixelY, posX, posY, runID):
    posX = posX + pixelsize * nbOfPixelX/2
    posY = posY + pixelsize * nbOfPixelY/2
    detectorWidth = nbOfPixelX / nbOfSlices
    projection = runID % nbOfProjections
    sliceOffset = int(math.floor(runID / nbOfProjections))
    sliceOffsetX = int(math.floor(detectorWidth*(nbOfSlices - 1) - detectorWidth*sliceOffset))
    pixelX = posX / pixelsize + sliceOffsetX-0.5
    pixelY = posY / pixelsize -0.5

    if(pixelX < 0 ):
        pixelX = 0
    if(pixelY < 0 ):
        pixelY = 0

    return [int(pixelX), int(pixelY), int(projection)]

def addSinogram(inputList,outputFile):
    outputFile = Path(outputFile)
    first = True
    adder = sitk.AddImageFilter()
    for fileSinogram in inputList:
        fileSinogram = Path(fileSinogram)
        if(fileSinogram.is_file()):
            sinReader = sitk.ImageFileReader()
            sinReader.SetFileName(str(fileSinogram))
            if(first):
                first = False
                sinogram = sinReader.Execute()
            else:
                source = sinReader.Execute()
                sinogram = adder.Execute(sinogram, source)

    if(not first):
        sinWriter = sitk.ImageFileWriter()
        sinWriter.SetFileName(str(outputFile))
        sinWriter.Execute(sinogram)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="generate macro files for Gate and run the simulation")
    parser.add_argument("Output", help="output directory")
    parser.add_argument("-p", "--params", action='append', type=lambda kv: kv.split("=", 1), dest='params')
    args = parser.parse_args()   
    params = dict(args.params) 
    main(args.Output, params)
