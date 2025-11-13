# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Copyright (c) 2025, Davide Stocco and Enrico Bertolazzi.                                        #
#                                                                                                 #
# The Pipal project is distributed under the MIT License.                                         #
#                                                                                                 #
# Davide Stocco                                                                 Enrico Bertolazzi #
# University of Trento                                                       University of Trento #
# e-mail: davide.stocco@unitn.it                               e-mail: enrico.bertolazzi@unitn.it #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

set(EIGEN_REQUIRED_VERSION 5.0.1)
cmake_policy(SET CMP0135 NEW)

# list(APPEND CMAKE_PREFIX_PATH "${PIPAL_THIRD_PARTY_DIR}")
find_package(
  Eigen3
  ${EIGEN_REQUIRED_VERSION}
  NO_MODULE
  QUIET
)

if(NOT TARGET Eigen3::Eigen)
  message(STATUS "Pipal: Did not find Eigen3 ${EIGEN_REQUIRED_VERSION} installed, downloading to "
    "${PIPAL_THIRD_PARTY_DIR}")

  include(FetchContent)
  set(FETCHCONTENT_BASE_DIR "${PIPAL_THIRD_PARTY_DIR}")
  fetchcontent_declare(
    Eigen3
    URL "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_REQUIRED_VERSION}/eigen-${EIGEN_REQUIRED_VERSION}.tar.gz"
  )

  set(EIGEN3_INSTALL_DIR "${FETCHCONTENT_BASE_DIR}/Eigen3")
  fetchcontent_makeavailable(Eigen3)
  set(EIGEN3_INCLUDE_DIR "${eigen3_SOURCE_DIR}")
  include_directories(${EIGEN3_INCLUDE_DIR})
else()
  get_target_property(EIGEN_INCLUDE_DIRS
    Eigen3::Eigen
    INTERFACE_INCLUDE_DIRECTORIES
  )
  message(STATUS "Pipal: Found Eigen3 installed in ${EIGEN_INCLUDE_DIRS}")
endif()
