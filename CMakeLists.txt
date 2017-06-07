#CMakeLists.txt for pco camera

SET(PCO_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdkPco" CACHE PATH "location of pco sdk")

#Sources
SET(pco_srcs src/PcoCamera.cpp src/PcoInterface.cpp src/PcoDetInfoCtrlObj.cpp
                src/PcoSyncCtrlObj.cpp src/PcoRoiCtrlObj.cpp
                src/PcoBufferCtrlObj.cpp src/PcoHwEventCtrlObj.cpp
		src/PcoCameraUtils.cpp)

FILE(STRINGS "VERSION" pco_ver)
ADD_LIBRARY(limapco SHARED
    ${pco_srcs}
)
TARGET_INCLUDE_DIRECTORIES(limapco PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
TARGET_INCLUDE_DIRECTORIES(limapco PUBLIC "${PCO_SDK_DIR}/include")


#LIBRARY
FIND_LIBRARY(PCO_LIB_CAM SC2_Cam ${PCO_SDK_DIR}/lib64)

TARGET_LINK_LIBRARIES(limapco limacore)
TARGET_LINK_LIBRARIES(limapco ${PCO_LIB_CAM})

INSTALL(TARGETS limapco DESTINATION lib)

IF(COMPILE_SIP)
    SET(NAME "pco")
    SET(INCLUDES)
    FILE(GLOB sipfiles
         RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/sip"
         "${CMAKE_CURRENT_SOURCE_DIR}/sip/*.sip")
    FOREACH(sipfile ${sipfiles})
        SET(INCLUDES
"${INCLUDES}
%Include ${sipfile}")
    ENDFOREACH()

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/sip/limamodules.sip.in sip/limapco.sip)
    SET(SIP_CONCAT_PARTS 1)
    SET(SIP_INCLUDES ${SIP_INCLUDES}
        "${CMAKE_SOURCE_DIR}/third-party/Processlib/sip"
        "${CMAKE_BINARY_DIR}/sip/core"
        "${CMAKE_SOURCE_DIR}/third-party/Processlib/tasks/sip"
        "${CMAKE_SOURCE_DIR}/common/sip"
        "${CMAKE_SOURCE_DIR}/hardware/sip"
        "${CMAKE_SOURCE_DIR}/control/sip"
        "${CMAKE_SOURCE_DIR}/control/software_operation/sip"
        "${CMAKE_CURRENT_SOURCE_DIR}/sip")
    set(SIP_DISABLE_FEATURES WITH_CONFIG)
    set(SIP_CHECK_EXC ON)
    ADD_SIP_PYTHON_MODULE(limapco ${CMAKE_CURRENT_BINARY_DIR}/sip/limapco.sip)
    TARGET_INCLUDE_DIRECTORIES(python_module_limapco PRIVATE
                               ${PYTHON_INCLUDE_DIRS}
                               "${CMAKE_SOURCE_DIR}/sip"
                               "${CMAKE_SOURCE_DIR}/sip/core")
    TARGET_LINK_LIBRARIES(python_module_limapco limapco)

    INSTALL(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/Pco")
ENDIF()
