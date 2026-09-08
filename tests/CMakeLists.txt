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
