set(PCO_INCLUDE_DIRS)
set(PCO_LIBRARIES)
set(PCO_DEFINITIONS)

set(PCO_SDKWIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPco" CACHE PATH "location of PCO Windows SDK")
set(PCO_SDKLIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPcoLin" CACHE PATH "location of PCO Linux SDK")

if(WIN32)
  find_path(PCO_INCLUDE_DIRS "PcoSdkVersion.h" ${PCO_SDKWIN_DIR})
  find_library(PCO_LIBRARIES SC2_Cam ${PCO_SDKWIN_DIR}/lib64)
  list(APPEND PCO_INCLUDE_DIRS ${PCO_INCLUDE_DIRS} ${PCO_INCLUDE_DIRS}/include)
else()
  #Current location of linux sdk, on the branch linux_dev.v123 the 09/06/17
  set(PCO_SDK_LIB_DIR "${PCO_SDKLIN_DIR}/pco_common/pco_lib" CACHE PATH "location of pco sdklib-lin")

  #set(SISO_LIB_DIR "${SISODIR5}/lib64" CACHE PATH "sisolib")
  set(SISO_LIB_DIR "/opt/SiliconSoftware/Runtime5.2.2/lib64" CACHE PATH "sisolib")

  find_path(PCO_INCLUDE_DIRS "PcoSdkVersion.h")
  find_library(PCO_CAM_ME4_LIB pcocam_me4 ${PCO_SDK_LIB_DIR})
  find_library(PCO_FILE_LIB pcofile ${PCO_SDK_LIB_DIR})
  find_library(PCO_LOG_LIB pcolog ${PCO_SDK_LIB_DIR})
  find_library(PCO_FUNC_LIB reorderfunc ${PCO_SDK_LIB_DIR})
  find_library(SISO_LIB_FGLIB5 fglib5 ${SISO_LIB_DIR})
  find_library(SISO_LIB_CLSERSIS clsersis ${SISO_LIB_DIR})
  find_library(SISO_LIB_HAPRT haprt ${SISO_LIB_DIR})
  list(APPEND PCO_LIBRARIES ${PCO_CAM_ME4_LIB} ${PCO_FILE_LIB} ${PCO_LOG_LIB} ${PCO_FUNC_LIB} ${SISO_LIB_FGLIB5} ${SISO_LIB_CLSERSIS} ${SISO_LIB_HAPRT})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCO DEFAULT_MSG
  PCO_LIBRARIES
  PCO_INCLUDE_DIRS
)
