add_executable(ase-containers-test
    ring_buffer_test.cpp
)

target_link_libraries(ase-containers-test
    PRIVATE
        ase::containers
        doctest::doctest
)

# DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN stand als #define in der Quelle und ist dort ein
# MACRO_DEFINITION_FORBIDDEN-Befund. Loeschen wuerde das main() entfernen und das Binary
# zerstoeren; hierher gehoert es ohnehin: der Define ist eine Eigenschaft des ZIELS, nicht des
# Textes. Genau EIN Ziel, genau EINE Uebersetzungseinheit — also genau ein main, wie vorher.
target_compile_definitions(ase-containers-test PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

add_test(NAME ase-containers-test COMMAND ase-containers-test)

set_target_properties(ase-containers-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# The module header containers.hpp through its own translation unit (usecase:101065 index entities
# by composite key): the one place that includes the umbrella and nothing else of the module.
# Its own target, because ase-containers-test above has exactly one unit and one main.
add_executable(ase-containers-umbrella-test
    containers_umbrella_test.cpp
)

target_link_libraries(ase-containers-umbrella-test
    PRIVATE
        ase::containers
        doctest::doctest
)

target_compile_definitions(ase-containers-umbrella-test PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

add_test(NAME ase-containers-umbrella-test COMMAND ase-containers-umbrella-test)

set_target_properties(ase-containers-umbrella-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
