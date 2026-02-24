# Fonction utilitaire
# Par Julien Rossignol
# Derniere modification : 22 juin 2017

import sys
import datetime
import os

mode = "Console"
logFile = ""

def SetLogMode(Logmode, outputDir = ""):
    global mode
    global logFile
    mode = Logmode
    logFile = outputDir

def printSection(EventStr):
    printStr("\n\r###### ###### - " +  str(datetime.datetime.now()) + " - " + str(EventStr))

def printEvent(EventStr):
    printStr("###### - " +  str(datetime.datetime.now()) + " - " +  str(EventStr))

def printWarning(warningStr):
    printStr("WARNING - " +  str(datetime.datetime.now()) + " - " + str(warningStr))

def printError(errorStr):
    printStr("ERROR - " +  str(datetime.datetime.now()) + " - " + str(errorStr))

def printStr(logStr):
    global mode
    global logFile
    if(mode == "Console"):
        print(logStr)
    elif(mode == "File"):
        with open(logFile, "a+") as f:
            f.write(logStr+"\n")
        f.close()  

def createDirectory(path):
    if(path.is_dir()):
        printWarning("Directory with path " + str(path) +  " already exists")
        return False
    else:
        try:
            path.mkdir()
            return True
        except:
            printError("Could not create specified directory with path " + str(path))	
            return False
        
def progressbar(it, count = 0, stride = 0, prefix = "", size = 50):
    """Progress bar iterator, adds a nice progress bar in the console.
       file iterators cannot know how many items will be, so count
       must be set. Otherwise an exception will be thrown
       modified from example at
       http://code.activestate.com/recipes/576986-progress-bar-for-console-programs-as-iterator/
       Tire du code du simulateur de Audrey Corbeil Therien
       """
    global mode
    if(count == 0):
        try:
            count = len(it)
        except:
            printWarning("Cannot find length, no progress bar")
            for i, item in enumerate(it):
                yield item

    if(stride == 0):
        try:
            if(count > 100):
                stride = count / 100
            else:
                stride = 1
        except:
            printWarning("Problem with stride??? exiting")
            exit(1)

    def _show(_i):
        global mode
        if(count != 0):
            x = int(size*_i/count)
            if(mode == "Console"):
                sys.stdout.write("%s [%s%s] %i/%i\r" % (prefix, "#"*x, "."*(size-x), _i, count))
                sys.stdout.flush()


    _show(0)
    for i, item in enumerate(it):
        yield item
        if(i % stride == 0 or (i-1) == count):
            _show(i+1)

    if(mode == "Console"):
        sys.stdout.write("%s [%s%s] %i/%i\r" % (prefix, "#"*size, "."*(0), count, count))
        #sys.stdout.write("\n")
        sys.stdout.flush()