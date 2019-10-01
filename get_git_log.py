import os
import datetime

strGit = "#define         PCO_GIT_VERSION \"$Id:       PCO %s at [%s] $\"\n"

fnVersion = "./include/PcoGitVersion.h"
fnSdkLin="./sdkPco/PcoSdkVersion.h"
fnSdkWin="./sdkPcoLin/include/PcoSdkVersion.h"

fOut = open(fnVersion, "w")

log = os.popen('git log -n 1 --date=iso --format=format:"rev[%ad] head[%h][%H] ref[%d]" HEAD || echo "ERROR"').read()
dt = '{0:%Y/%m/%d %H:%M:%S}'.format(datetime.datetime.now())
line = strGit % (log, dt)
fOut.write(line)

try:
    fIn = open(fnSdkLin, "r")
    lines = fIn.readlines()
    line = lines[0]
    fIn.close()
except:
    line = "#define PCO_SDK_LIN_VERSION \"$Id: PCOSDK_LIN [not defined] $\""
fOut.write(line)

try:
    fIn = open(fnSdkWin, "r")
    lines = fIn.readlines()
    line = lines[0]
    fIn.close()
except:
    line = "#define PCO_SDK_WIN_VERSION \"$Id: PCOSDK_WIN [not defined] $\""
fOut.write(line)

line = strGit % (log, dt)


fOut.close()
