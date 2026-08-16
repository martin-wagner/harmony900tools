include(FetchContent)

FetchContent_Declare(
    IRremoteESP8266
    GIT_REPOSITORY https://github.com/crankyoldgit/IRremoteESP8266.git
    GIT_TAG 8833210 #v2.9.0
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/IRremoteESP8266
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/IRremoteESP8266
)

FetchContent_GetProperties(IRremoteESP8266)
if (NOT IRremoteESP8266_POPULATED)
    FetchContent_MakeAvailable(IRremoteESP8266)
endif()

# IRremoteESP8266 does not provide a suitable PC/CMake build.
# Build its sources directly in UNIT_TEST mode. This disables the
# Arduino/ESP hardware-specific parts of IRrecv.cpp.
file(GLOB IRREMOTEESP8266_SOURCES
    "${irremoteesp8266_SOURCE_DIR}/src/*.cpp"
)

add_library(IRremoteESP8266 STATIC
    ${IRREMOTEESP8266_SOURCES}
)

target_compile_features(IRremoteESP8266
    PUBLIC
        cxx_std_17
)

target_compile_definitions(IRremoteESP8266
    PRIVATE
        UNIT_TEST # we are not an ESP8266. Nice of the project to have unit tests :-) I don't even want to know what this is doing...
)

target_include_directories(IRremoteESP8266
    PUBLIC
        "${irremoteesp8266_SOURCE_DIR}/src"
    PRIVATE
        "${irremoteesp8266_SOURCE_DIR}/test"
)