cmake_minimum_required(VERSION 3.20)

project(ase-containers
    VERSION 1.0.0
    DESCRIPTION "ASE Containers - High-performance data structures for game engines"
    LANGUAGES CXX
)

# Header-only library (Layer 0 Foundation)
add_library(ase-containers INTERFACE)
add_library(ase::containers ALIAS ase-containers)

target_include_directories(ase-containers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(ase-containers INTERFACE cxx_std_20)

# Tests
if(ASE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Install
install(TARGETS ase-containers
    EXPORT ase-containers-targets
)

install(DIRECTORY include/ase
    DESTINATION include
)
