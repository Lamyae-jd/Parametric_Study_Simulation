import uproot
import matplotlib.pyplot as plt
import numpy as np

inputFileName = "outputCrystal2.root"
outputFileName = "sortie"

def ImageReconstruction(inputFileName, outputFileName):
    # Open the ROOT file and get the tree
    file = uproot.open(inputFileName)
    tree = file["Hits"]

    # Get the positions and energies of detected photons
    x_positions = tree.array("posX")
    y_positions = tree.array("posY")
    energies = tree.array("edep")

    # Create a 2D histogram for image reconstruction
    image_hist, x_edges, y_edges = np.histogram2d(x_positions, y_positions, bins=100, range=[[-10, 10], [-10, 10]], weights=energies)

    # Plot the image
    plt.imshow(image_hist.T, origin='lower', extent=[x_edges[0], x_edges[-1], y_edges[0], y_edges[-1]])
    plt.colorbar(label='Energy')
    plt.xlabel('X Position')
    plt.ylabel('Y Position')
    plt.title('Image Reconstruction')
    plt.savefig(outputFileName)
    plt.show()

# Usage
inputFileName = "input.root"
outputFileName = "output.png"

ImageReconstruction(inputFileName, outputFileName)
