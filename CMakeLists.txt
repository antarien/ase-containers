cmake_minimum_required(VERSION 3.20)

#[[
ASE FOUNDATION BUILD CONFIGURATION

@file        CMakeLists.txt
@brief       Build configuration for ase-containers
@description Containers library - High-performance data structures.
             RingBuffer, SparseSet, and other game engine containers.

@module      ase-containers
@layer       0 (Foundation)
@created     2025-12-15
@modified    2026-01-30
@version     00.00.04.00004 [seed]

CMAKE MODULE COMPLIANCE

[ ] cmake_minimum_required(VERSION 3.20) as FIRST line
[ ] project() with VERSION and LANGUAGES CXX
[ ] message(STATUS) for configuration output
[ ] add_library() with INTERFACE for header-only
[ ] add_library(ase::{module} ALIAS ...) for namespace
[ ] target_include_directories() with INTERFACE and generator expressions
[ ] target_compile_features() with cxx_std_20
[ ] NO external dependencies (Layer 0)
[ ] if(ASE_BUILD_TESTS) block for tests
[ ] add_subdirectory(tests) inside test block
[ ] include(GNUInstallDirs) for installation
[ ] install(DIRECTORY include/ ...) for headers
]]

# Parse local VERSION (SSOT) if not building from root
if(NOT DEFINED ASE_CONTAINERS_VERSION)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
        message(FATAL_ERROR "VERSION file not found: ${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    endif()
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" _VERSION_LINES)
    foreach(_LINE ${_VERSION_LINES})
        if(_LINE MATCHES "^ASE_CONTAINERS=(.*)$")
            set(ASE_CONTAINERS_VERSION "${CMAKE_MATCH_1}")
        endif()
    endforeach()
    if(NOT DEFINED ASE_CONTAINERS_VERSION)
        message(FATAL_ERROR "ASE_CONTAINERS version not found in VERSION file")
    endif()
endif()

project(ase-containers VERSION ${ASE_CONTAINERS_VERSION} LANGUAGES CXX)

# Generate version.hpp from template (SSOT)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ase/containers/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/ase/containers/version.hpp"
    @ONLY
)

# ==============================================================================
# BUILD CONFIGURATION
# ==============================================================================



message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

#[[
HEADER-ONLY LIBRARY
Game engine containers: RingBuffer, SparseSet, ObjectPool, etc.
]]
add_library(${PROJECT_NAME} INTERFACE)
add_library(ase::containers ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} INTERFACE
    #[[
    PUBLIC API
    Main entry point header that includes all container types.
    ]]
    include/ase/containers/containers.hpp

    #[[
    TYPES
    Type definitions and constants for containers module.
    ]]
    include/ase/containers/types.hpp

    #[[
    DATA STRUCTURES
    High-performance container implementations for game engines.
    ]]
    include/ase/containers/array.hpp
    include/ase/containers/hash_map.hpp
    include/ase/containers/ordered.hpp
    include/ase/containers/ring_buffer.hpp
    include/ase/containers/vector.hpp

    #[[
    HASHING
    Hash functors the containers above are instantiated with.
    ]]
    include/ase/containers/int_hash.hpp
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

#[[ VERBOTEN: No dependencies allowed in Layer 0! ]]

if(ASE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
