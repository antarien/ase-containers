add_executable(ase-containers-test
    ring_buffer_test.cpp
)

target_link_libraries(ase-containers-test
    PRIVATE
        ase::containers
        doctest::doctest
)

add_test(NAME ase-containers-test COMMAND ase-containers-test)

set_target_properties(ase-containers-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
