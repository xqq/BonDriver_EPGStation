cmake_minimum_required(VERSION 3.13)
project(BonDriver_EPGStation LANGUAGES C CXX)

# main project checking
# determine if BonDriver_EPGStation is built as a subproject (using add_subdirectory) or if it is the main project
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

option(BONDRIVER_EPGSTATION_BUILD_TEST "Build test program." ${MAIN_PROJECT})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(thirdparty/json)
add_subdirectory(thirdparty/yaml-cpp)

add_library(BonDriver_EPGStation
    SHARED
        include/IBonDriver.h
        include/IBonDriver2.h
        include/min_win32_typedef.hpp
        include/export.hpp
        src/bon_driver.cpp
        src/bon_driver.hpp
        src/config.cpp
        src/config.hpp
        src/library.cpp
        src/library.hpp
        src/log.cpp
        src/log.hpp
)

# Remove "lib" prefix for dll filename
if(WIN32)
    set_target_properties(BonDriver_EPGStation PROPERTIES PREFIX "")
endif()

# Platform-specific compiler macro / flags
if(MSVC)
    # For MSVC, disable CRT secure warnings
    target_compile_definitions(BonDriver_EPGStation
        PRIVATE
            _CRT_SECURE_NO_WARNINGS=1
    )
elseif(MINGW)
    # For MinGW, static linking runtime library
    target_compile_options(BonDriver_EPGStation
        PRIVATE
            -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive -Wl,-Bdynamic
    )
endif()

# Macros
target_compile_definitions(BonDriver_EPGStation
    PRIVATE
        BONDRIVER_EPGSTATION_EXPORTS=1
)

# Include directories
target_include_directories(BonDriver_EPGStation
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link directories
target_link_directories(BonDriver_EPGStation
    PRIVATE
)

# Linking
target_link_libraries(BonDriver_EPGStation
    PRIVATE
        nlohmann_json::nlohmann_json
        yaml-cpp
)

if(MSVC)
    # library linkage for MSVC
    target_link_libraries(BonDriver_EPGStation
        PRIVATE
            # libraries
    )
elseif(MINGW)
    # library linkage for MinGW: Enable full static link
    target_link_libraries(BonDriver_EPGStation
        PRIVATE
            -Wl,-Bstatic
            # libraries
            -Wl,-Bdynamic
    )
else()
    # library linkage for other platforms: dynamic link by default
    target_link_libraries(BonDriver_EPGStation
        PRIVATE
            # libraries
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # pthread is needed for gcc
    target_link_libraries(BonDriver_EPGStation
        PRIVATE
            pthread
    )
endif()

if(WIN32)
    target_link_libraries(BonDriver_EPGStation
        PRIVATE
            Imm32
            Winmm
            Version
            SetupAPI
            Ws2_32
    )
endif()

if(BONDRIVER_EPGSTATION_BUILD_TEST)
    add_subdirectory(test)
endif()
