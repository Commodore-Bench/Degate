#####################################################################
# This file is part of the IC reverse engineering tool Degate.
#
# Copyright 2008, 2009, 2010 by Martin Schobert
# Copyright 2019-2020 Dorian Bachelot
#
# Degate is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# Degate is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with degate. If not, see <http://www.gnu.org/licenses/>.
#
#####################################################################

cmake_policy(SET CMP0007 NEW)

# Read version file
file(READ "${PROJECT_SOURCE_DIR}/VERSION" VERSION_FILE_CONTENT)
string(TIMESTAMP CURRENT_DATE "%Y-%m-%d")

# Create the list
STRING(REGEX REPLACE "\n" ";" VERSION_FILE_CONTENT "${VERSION_FILE_CONTENT}")

# Clear empty list entries
STRING(REGEX REPLACE ";;" ";" VERSION_FILE_CONTENT "${VERSION_FILE_CONTENT}")

# Remove lines that start with a '#'
foreach(LINE ${VERSION_FILE_CONTENT})
    if("${LINE}" MATCHES "^(#.*|\\n)")
        list(REMOVE_ITEM VERSION_FILE_CONTENT "${LINE}")
        continue()
    endif()
endforeach()

# Read the Degate version
list(GET VERSION_FILE_CONTENT 0 DEGATE_VERSION)

# Read the Degate version release date
list(GET VERSION_FILE_CONTENT 1 DEGATE_RELEASE_DATE)

# Print information
if("${PROJECT_NAME}" STREQUAL "")
    # If the project name is not set, then we are in the build process
    execute_process(COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --red --bold
            "======================================="
            "\t Degate version: ${DEGATE_VERSION}"
            "\t Release date:   ${DEGATE_RELEASE_DATE}"
            "\t Build date:     ${CURRENT_DATE}"
            "\t OS:             ${CMAKE_SYSTEM_NAME}"
            "\t Configuration:  ${CONFIGURATION}"
            "=======================================")
else()
    # Otherwise we are in the CMakeLists load/reload process
    message(STATUS "Degate version: ${DEGATE_VERSION}")
    message(STATUS "Release date: ${DEGATE_RELEASE_DATE}")
endif()

# Format version to Qt compatible one
string(REPLACE "alpha" "0" DEGATE_VERSION_STR "${DEGATE_VERSION}")
string(REPLACE "beta" "1" DEGATE_VERSION_STR "${DEGATE_VERSION_STR}")
string(REPLACE "rc" "2" DEGATE_VERSION_STR "${DEGATE_VERSION_STR}")

# Set if the version is a nightly
if("${DEGATE_VERSION}" MATCHES "(-)")
    set(VERSION_TYPE "nightly")
else()
    set(VERSION_TYPE "release")
    set(DEGATE_VERSION_STR "${DEGATE_VERSION_STR}-3.0")
endif()

# Configure version file
configure_file("${PROJECT_SOURCE_DIR}/etc/config/Version.config" "${PROJECT_SOURCE_DIR}/src/Core/Version.h")

# Configure doxyfile
configure_file("${PROJECT_SOURCE_DIR}/doc/config/Doxyfile.config" "${PROJECT_SOURCE_DIR}/doc/config/DoxyFile")

# If the release date is "unreleased" then take the current date (build date)
if(DEGATE_RELEASE_DATE MATCHES "unreleased")
    set(DEGATE_RELEASE_DATE "${CURRENT_DATE}")
endif()

# Skip some variables (Qt specific)
set(ApplicationsDir "@ApplicationsDir@")

# Set the platform variable
if("${CMAKE_SYSTEM_NAME}" MATCHES "(Windows)")
	set(PLATFORM "windows")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "(Linux)")
	set(PLATFORM "linux")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "(Darwin)")
    set(PLATFORM "macos")
endif()

# Configure files
configure_file("${PROJECT_SOURCE_DIR}/etc/installer/packages/Degate/meta/package.config" "${PROJECT_SOURCE_DIR}/etc/installer/packages/Degate/meta/package.xml")
configure_file("${PROJECT_SOURCE_DIR}/etc/installer/config/config.config" "${PROJECT_SOURCE_DIR}/etc/installer/config/config.xml")