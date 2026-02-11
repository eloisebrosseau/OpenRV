#
# Copyright (C) 2023  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

INCLUDE(cxx_defaults)

SET(_target
    "RVMath"
)

SET(_sources
    LogC.cpp
)

ADD_LIBRARY(
  ${_target} STATIC
  ${_sources}
)

TARGET_INCLUDE_DIRECTORIES(
  ${_target}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

IF(MSVC)
  TARGET_COMPILE_OPTIONS(
    ${_target}
    PRIVATE /W4
  )
ELSE()
  TARGET_COMPILE_OPTIONS(
    ${_target}
    PRIVATE -Wall -Wextra -Wpedantic -Werror
  )
ENDIF()

RV_STAGE(TYPE "LIBRARY" TARGET ${_target})
