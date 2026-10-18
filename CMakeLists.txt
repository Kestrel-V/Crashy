cmake_minimum_required(VERSION 3.22)

project(Crashy LANGUAGES NONE)

include(ExternalProject)

set(CUBEMX_DIR 
    "${CMAKE_SOURCE_DIR}/firmware/platform/stm32/cubemx"
)

# Build both cores (CM7 and CM4) using the same firmware project
foreach(CORE IN ITEMS CM7 CM4)

    ExternalProject_Add(crashy_${CORE}
        SOURCE_DIR "${CUBEMX_DIR}/${CORE}"
        BINARY_DIR "${CMAKE_BINARY_DIR}/${CORE}"

        CMAKE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CUBEMX_DIR}/gcc-arm-none-eabi.cmake"
            "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON"
    
        INSTALL_COMMAND ""
        BUILD_ALWAYS TRUE
        EXCLUDE_FROM_ALL TRUE
    )

endforeach()

# Build both cores (CM7 and CM4) using the default build command
add_custom_target(stm32_firmware ALL
    DEPENDS crashy_CM7 crashy_CM4
)
