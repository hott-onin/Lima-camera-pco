###########################################################################
# This file is part of LImA, a Library for Image Acquisition
#
#  Copyright (C) : 2009-2017
#  European Synchrotron Radiation Facility
#  BP 220, Grenoble 38043
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

IF(WIN32)
SET(NAME "pco")

SET(PCO_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPco" CACHE PATH "location of pco sdk")

#Sources
SET(${NAME}_srcs src/PcoCamera.cpp src/PcoInterface.cpp src/PcoDetInfoCtrlObj.cpp
	                src/PcoSyncCtrlObj.cpp src/PcoRoiCtrlObj.cpp
	                src/PcoBufferCtrlObj.cpp src/PcoHwEventCtrlObj.cpp
			src/PcoCameraUtils.cpp src/PcoCameraSdk.cpp)


ADD_LIBRARY(lima${NAME} SHARED ${${NAME}_srcs})

TARGET_INCLUDE_DIRECTORIES(lima${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
TARGET_INCLUDE_DIRECTORIES(lima${NAME} PUBLIC "${PCO_SDK_DIR}/include")
TARGET_INCLUDE_DIRECTORIES(lima${NAME} PUBLIC "${PCO_SDK_DIR}")

#LIBRARY
FIND_LIBRARY(PCO_LIB_CAM SC2_Cam ${PCO_SDK_DIR}/lib64)

TARGET_LINK_LIBRARIES(lima${NAME} limacore)
TARGET_LINK_LIBRARIES(lima${NAME} ${PCO_LIB_CAM})

limatools_set_library_soversion(lima${NAME} "VERSION")
SET_TARGET_PROPERTIES(lima${NAME} PROPERTIES PREFIX "lib")
INSTALL(TARGETS lima${NAME} DESTINATION lib)

IF(LIMA_ENABLE_PYTHON)
    limatools_run_sip_for_camera(${NAME})
    INSTALL(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/Pco")
ENDIF()
ENDIF()