#
# Copyright (C) 2023  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

INCLUDE(cxx_defaults)

SET(_target
    "NDI"
)

SET(_sources
    NDIModule.cpp NDIVideoDevice.cpp NDIOutput.cpp
)

ADD_LIBRARY(
  ${_target} SHARED
  ${_sources}
)

FIND_PACKAGE(NDI_SDK REQUIRED)

TARGET_INCLUDE_DIRECTORIES(
  ${_target}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

TARGET_LINK_LIBRARIES(
  ${_target}
  PRIVATE NDI_SDK::NDI_SDK Boost::program_options Boost::thread TwkApp TwkGLF TwkUtil
)

IF(RV_TARGET_DARWIN)
  TARGET_LINK_LIBRARIES(
    ${_target}
    PRIVATE "-framework Foundation" "-framework IOKit"
  )
ELSEIF(RV_TARGET_WINDOWS)
  TARGET_LINK_LIBRARIES(
    ${_target}
    PRIVATE winmm Setupapi Netapi32 Shlwapi
  )
ENDIF()

ADD_CUSTOM_COMMAND(
  TARGET ${_target}
  POST_BUILD
  COMMENT "Installing NDI SDK libs into ${RV_STAGE_LIB_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy ${NDI_SDK_LIBRARY} ${RV_STAGE_LIB_DIR}
)

RV_STAGE(TYPE "OUTPUT_PLUGIN" TARGET ${_target})
