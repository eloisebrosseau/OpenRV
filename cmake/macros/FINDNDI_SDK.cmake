#
# Copyright (C) 2023  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

# Find the NDI SDK headers and libraries.
#
# Once done this will define: 
#    NDI_SDK_FOUND    - true if NDI SDK has been found
#    NDI_SDK::NDI_SDK - Imported target
#
# Note: Set NDI_SDK_ROOT to specify an additional directory to search.

FIND_PATH(
  NDI_SDK_INCLUDE_DIR
  NAMES Processing.NDI.Lib.h
  PATHS "${NDI_SDK_ROOT}/include" "$ENV{NDI_SDK_ROOT}/include" "/Library/NDI SDK for Apple/include"
  DOC "NDI_SDK include directory"
)
MARK_AS_ADVANCED(NDI_SDK_INCLUDE_DIR)

FIND_LIBRARY(
  NDI_SDK_LIBRARY
  NAMES ndi
  PATHS "${NDI_SDK_ROOT}/lib" "$ENV{NDI_SDK_ROOT}/lib" "/Library/NDI SDK for Apple/lib/macOS"
  DOC "NDI_SDK library"
)
MARK_AS_ADVANCED(NDI_SDK_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  NDI_SDK
  REQUIRED_VARS NDI_SDK_INCLUDE_DIR NDI_SDK_LIBRARY
  VERSION_VAR NDI_SDK_VERSION
)

IF(NDI_SDK_FOUND)
  IF(NOT TARGET NDI_SDK::NDI_SDK)
    ADD_LIBRARY(NDI_SDK::NDI_SDK UNKNOWN IMPORTED)
    SET_TARGET_PROPERTIES(
      NDI_SDK::NDI_SDK
      PROPERTIES IMPORTED_LOCATION "${NDI_SDK_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${NDI_SDK_INCLUDE_DIR}"
    )
  ENDIF()
ENDIF()
