# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Copyright (c) 2025, Davide Stocco and Enrico Bertolazzi.                                        #                                                              #
#                                                                                                 #
# The Pipal project is distributed under the MIT License.                                         #
#                                                                                                 #
# Davide Stocco                                                                 Enrico Bertolazzi #
# University of Trento                                                       University of Trento #
# e-mail: davide.stocco@unitn.it                               e-mail: enrico.bertolazzi@unitn.it #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

cmake_minimum_required(VERSION 3.14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(PipalVersion)
get_version_from_git()

project(Pipal
  VERSION ${PIPAL_VERSION}
  DESCRIPTION "Penalty Interior-Point ALgorithm"
  HOMEPAGE_URL "https://github.com/StoccoDavide/Pipal"
  LANGUAGES CXX
)

if(NOT PIPAL_VERSION_PRERELEASE)
  set(PIPAL_FULL_VERSION "${PIPAL_VERSION}")
  write_version_file()
else()
  set(PIPAL_FULL_VERSION "${PIPAL_VERSION}-${PIPAL_VERSION_PRERELEASE}")
endif()
message(STATUS "Pipal: version ${PIPAL_FULL_VERSION}")

set(PIPAL_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/lib")
  message(STATUS "Pipal: CMAKE_INSTALL_PREFIX not specified, defaulting to ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Dependencies
# ==============================================================
# Dependencies
# ==============================================================

include(FetchContent)

# Imposta la directory base dove scaricare i contenuti
set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib3rd")

# --------------------------------------------------------------
# Eigen 5.0.0
# --------------------------------------------------------------
# Usa il mirror ufficiale GitLab (Eigen 5.0.0)
FetchContent_Declare(
  Eigen3
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 5.0.0         # versione stabile (puoi usare anche "master")
  GIT_SHALLOW TRUE
)

# Crea la directory dove installare gli header di Eigen
set(EIGEN3_INSTALL_DIR "${FETCHCONTENT_BASE_DIR}/Eigen3")

# Scarica Eigen
FetchContent_MakeAvailable(Eigen3)

# Aggiungi al path include
set(EIGEN3_INCLUDE_DIR "${eigen3_SOURCE_DIR}")

# --------------------------------------------------------------
# Include directories per il progetto
# --------------------------------------------------------------
include_directories( ${EIGEN3_INCLUDE_DIR} )

message(STATUS "Eigen include path: ${EIGEN3_INCLUDE_DIR}")

# Library definition
add_library(Pipal INTERFACE)
add_library(Pipal::Pipal ALIAS Pipal)

target_include_directories(Pipal INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# INFO: Ensure this options are only available to developers. In theory we could leave them available,
# but the call to `target_sources` breaks `fetchcontent` compatibility due to absolute paths being
# added to `INTERFACE_SOURCES`. I tried solving it, but it seems to be poorly documented, supported, etc.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
    message(STATUS
      "Pipal: "
      "CMAKE_BUILD_TYPE not specified, defaulting to ${CMAKE_BUILD_TYPE}"
    )
  endif()

  option(PIPAL_BUILD_TESTS "Build tests" OFF)
  option(PIPAL_BUILD_EXAMPLES "Build examples" OFF)

  file(GLOB_RECURSE HEADER_FILES_HH "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h*")
  foreach(HEADER_FILE IN LISTS HEADER_FILES_HH)
    file(
      RELATIVE_PATH
      RELATIVE_PATH_HEADER_FILE
      ${CMAKE_CURRENT_SOURCE_DIR} ${HEADER_FILE}
    )
    list(APPEND RELATIVE_HEADER_FILES ${RELATIVE_PATH_HEADER_FILE})
  endforeach()
  target_sources(Pipal INTERFACE $<BUILD_INTERFACE:${RELATIVE_HEADER_FILES}>)

  if(PIPAL_BUILD_TESTS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
  endif()

  if(PIPAL_BUILD_EXAMPLES)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
  endif()

endif()

if(MSVC)
  target_compile_options(Pipal INTERFACE /W4 /WX)
else()
  target_compile_options(Pipal INTERFACE -Wall -Wextra -pedantic -Werror) # -Wno-narrowing
endif()

# Installation
install(TARGETS Pipal EXPORT PipalTargets)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

include(CMakePackageConfigHelpers)

export(EXPORT PipalTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/PipalTargets.cmake
  NAMESPACE Pipal::
)

install(EXPORT PipalTargets
  FILE PipalTargets.cmake
  NAMESPACE Pipal::
  DESTINATION lib/cmake/Pipal
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PipalConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/PipalConfig.cmake
  INSTALL_DESTINATION lib/cmake/Pipal
)

write_basic_package_version_file(
  PipalConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/PipalConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/PipalConfigVersion.cmake
  DESTINATION lib/cmake/Pipal
)
