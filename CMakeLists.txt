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

set(NAME "pco")
set(PCO_SDKWIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPco" CACHE PATH "location of pco sdk-win")
set(PCO_SDKLIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPcoLin" CACHE PATH "location of pco sdk-lin")

#============================================================================
#============================================================================

IF(WIN32)
    #Sources
    SET(${NAME}_srcs
      src/PcoBinCtrlObj.cpp
      src/PcoBufferCtrlObj.cpp
      src/PcoCamera.cpp
      src/PcoCameraSdk.cpp
      src/PcoCameraSdkWin.cpp
      src/PcoCameraSip.cpp
      src/PcoCameraUtils.cpp
      src/PcoDetInfoCtrlObj.cpp
      src/PcoHwEventCtrlObj.cpp
      src/PcoInterface.cpp
      src/PcoRoiCtrlObj.cpp
      src/PcoSyncCtrlObj.cpp
    )

    ADD_LIBRARY(lima${NAME} SHARED ${${NAME}_srcs})

    TARGET_INCLUDE_DIRECTORIES(lima${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    TARGET_INCLUDE_DIRECTORIES(lima${NAME} PUBLIC "${PCO_SDKWIN_DIR}/include")
    TARGET_INCLUDE_DIRECTORIES(lima${NAME} PUBLIC "${PCO_SDKWIN_DIR}")

    #LIBRARY
    FIND_LIBRARY(PCO_LIB_CAM SC2_Cam ${PCO_SDKWIN_DIR}/lib64)

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

#============================================================================
#============================================================================
if(UNIX)

    #Current location of linux sdk, on the branch linux_dev.v123 the 09/06/17
    set(PCO_SDK_LIB_DIR "${PCO_SDKLIN_DIR}/pco_common/pco_lib" CACHE PATH "location of pco sdklib-lin")

    #set(SISO_LIB_DIR "${SISODIR5}/lib64" CACHE PATH "sisolib")
    set(SISO_LIB_DIR "/opt/SiliconSoftware/Runtime5.2.2/lib64" CACHE PATH "sisolib")

    #Sources
    set(${NAME}_srcs 
        src/PcoBinCtrlObj.cpp  
        src/PcoBufferCtrlObj.cpp  
        src/PcoCamera.cpp  
        src/PcoCameraSdk.cpp  
        src/PcoCameraSdkLin.cpp  
        src/PcoCameraSip.cpp  
        src/PcoCameraUtils.cpp  
        src/PcoDetInfoCtrlObj.cpp  
        src/PcoHwEventCtrlObj.cpp  
        src/PcoInterface.cpp  
        src/PcoRoiCtrlObj.cpp  
        src/PcoSyncCtrlObj.cpp  
    )

    add_library(lima${NAME} SHARED ${${NAME}_srcs})

    target_include_directories(lima${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_include_directories(lima${NAME} PUBLIC "${PCO_SDKLIN_DIR}/include")
    target_include_directories(lima${NAME} PUBLIC "${PCO_SDKWIN_DIR}/include")

    #LIBRARIES
#	libfglib5.so => /opt/SiliconSoftware/Runtime5.2.2/lib/libfglib5.so (0x00007f4c7459e000)
#	libclsersis.so => /opt/SiliconSoftware/Runtime5.2.2/lib/libclsersis.so (0x00007f4c74496000)
#	libhaprt.so => /opt/SiliconSoftware/Runtime5.2.2/lib/libhaprt.so (0x00007f4c7435f000)

#    link_directories(${SISO_LIB_DIR})

    # ----- find_library
    find_library(PCO_CAM_ME4_LIB pcocam_me4 ${PCO_SDK_LIB_DIR})
    find_library(PCO_FILE_LIB pcofile ${PCO_SDK_LIB_DIR})
    find_library(PCO_LOG_LIB pcolog ${PCO_SDK_LIB_DIR})
    find_library(PCO_FUNC_LIB reorderfunc ${PCO_SDK_LIB_DIR})
  
    find_library(SISO_LIB_FGLIB5 fglib5 ${SISO_LIB_DIR})
    find_library(SISO_LIB_CLSERSIS clsersis ${SISO_LIB_DIR})
    find_library(SISO_LIB_HAPRT haprt ${SISO_LIB_DIR})

    find_library(PCO_PTHREAD_LIB pthread)

    # ----- target_link_libraries
    target_link_libraries(lima${NAME} limacore)

    target_link_libraries(lima${NAME} ${PCO_CAM_ME4_LIB})
    target_link_libraries(lima${NAME} ${PCO_FILE_LIB})
    target_link_libraries(lima${NAME} ${PCO_LOG_LIB})
    target_link_libraries(lima${NAME} ${PCO_FUNC_LIB})

    target_link_libraries(lima${NAME} ${SISO_LIB_FGLIB5})
    target_link_libraries(lima${NAME} ${SISO_LIB_CLSERSIS})
    target_link_libraries(lima${NAME} ${SISO_LIB_HAPRT})
    
    target_link_libraries(lima${NAME} ${PCO_PTHREAD_LIB})

    limatools_set_library_soversion(lima${NAME} "VERSION")

    install(TARGETS lima${NAME} LIBRARY DESTINATION lib)


    if(LIMA_ENABLE_PYTHON)
        limatools_run_sip_for_camera(${NAME})
        install(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/Pco")
    endif()
endif()

#============================================================================
#============================================================================

