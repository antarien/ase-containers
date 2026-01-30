cmake_minimum_required(VERSION 3.20)
project(ase-containers VERSION 1.0.0 LANGUAGES CXX)

#[[
ASE FOUNDATION BUILD CONFIGURATION

@file        CMakeLists.txt
@brief       Build configuration for ase-containers
@description Containers library - High-performance data structures.
             RingBuffer, SparseSet, and other game engine containers.

@module      ase-containers
@layer       0 (Foundation)
@created     2025-01-01
@modified    2025-01-30
@version     1.0.0

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

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

#[[
HEADER-ONLY LIBRARY
Game engine containers: RingBuffer, SparseSet, ObjectPool, etc.
]]
add_library(${PROJECT_NAME} INTERFACE)
add_library(ase::containers ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
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
