###########################################################################
# This file is part of LImA, a Library for Image Acquisition
#
#  Copyright (C) : 2009-2017
#  European Synchrotron Radiation Facility
#  CS40220 38043 Grenoble Cedex 9 
#  FRANCE
# 
#  Contact: lima@esrf.fr
# 
#  This is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
# 
#  This software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

set(PCO_INCLUDE_DIRS)
set(PCO_LIBRARIES)
set(PCO_DEFINITIONS)

set(PCO_SDKWIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPco" CACHE PATH "location of PCO Windows SDK")
set(PCO_SDKLIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPcoLin" CACHE PATH "location of PCO Linux SDK")

if(WIN32)
  find_path(PCO_INCLUDE_DIRS "PcoSdkVersion.h" ${PCO_SDKWIN_DIR})
  find_library(PCO_LIBRARIES NAMES SC2_Cam.lib HINTS ${PCO_SDKWIN_DIR}/lib64)
else()
  set(PCO_SDK_LIB_DIR "${PCO_SDKLIN_DIR}/pco_common/pco_lib" CACHE PATH "location of pco sdklib-lin")

  set(SISO_DIR "$ENV{SISODIR5}" CACHE PATH "SISO BASE DIR")
  set(SISO_LIB_DIR "${SISO_DIR}/lib" CACHE PATH "SISO LIB DIR")

list(APPEND PCO_INCLUDE_DIRS
  ${SISO_DIR}/include
  ${PCO_SDKLIN_DIR}/include
  ${PCO_SDKLIN_DIR}
  ${PCO_SDKLIN_DIR}/pco_common/pco_include
  ${PCO_SDKLIN_DIR}/pco_common/pco_classes
  ${PCO_SDKLIN_DIR}/pco_me4/pco_classes
  ${PCO_SDKWIN_DIR}/include
  )


 set(LP1)
 set(LP2)
 set(LP3)
 set(LP4)
 
 find_library(LP1 pcocam_me4 HINTS ${PCO_SDK_LIB_DIR})
 find_library(LP2 pcofile HINTS ${PCO_SDK_LIB_DIR})
 find_library(LP3 pcolog HINTS ${PCO_SDK_LIB_DIR})
 find_library(LP4 reorderfunc HINTS ${PCO_SDK_LIB_DIR})

 set(LS1)
 set(LS2)
 set(LS3)
 find_library(LS1 fglib5 HINTS ${SISO_LIB_DIR})
 find_library(LS2 clsersis HINTS ${SISO_LIB_DIR})
 find_library(LS3 haprt HINTS ${SISO_LIB_DIR})
 
 
 list(APPEND PCO_LIBRARIES 
    ${LP1} ${LP2} ${LP3} ${LP4}
    ${LS1} ${LS2} ${LS3}
    )

endif()


message("PCO_LIBRARIES ============================================")
message("PCO_INCLUDE_DIRS: ${PCO_INCLUDE_DIRS}")
message("PCO_LIBRARIES: ${PCO_LIBRARIES}")
message("==========================================================")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCO DEFAULT_MSG
  PCO_LIBRARIES
  PCO_INCLUDE_DIRS
)

list(APPEND PCO_DEFINITIONS WITH_GIT_VERSION)


