cmake_minimum_required(VERSION 3.12)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

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

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
endif()


set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Disable yaml-cpp contrib")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp build tools")
set(YAML_MSVC_SHARED_RT OFF CACHE BOOL "Use static linked runtime for yaml-cpp")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build cpr as static library")
set(BUILD_CPR_TESTS OFF CACHE BOOL "Disable cpr tests building")

set(CURL_STATICLIB true)

if(WIN32)
    set(USE_OPENSSL OFF)
    set(USE_WINSSL ON)
    add_definitions(-DCURL_STATICLIB)
endif()

find_package(CURL)
if(CURL_FOUND)
    set(USE_SYSTEM_CURL ON CACHE BOOL "Use the system curl for faster builds")
endif()


add_subdirectory(thirdparty/cpr)
add_subdirectory(thirdparty/json)
add_subdirectory(thirdparty/yaml-cpp)

add_library(BonDriver_EPGStation
    SHARED
        include/IBonDriver.h
        include/IBonDriver2.h
        include/min_win32_typedef.hpp
        include/export.hpp
        src/blocking_buffer.cpp
        src/blocking_buffer.hpp
        src/bon_driver.cpp
        src/bon_driver.hpp
        src/config.cpp
        src/config.hpp
        src/epgstation_api.cpp
        src/epgstation_api.hpp
        src/epgstation_models.hpp
        src/epgstation_models_deserialize.hpp
        src/library.cpp
        src/library.hpp
        src/log.cpp
        src/log.hpp
        src/noncopyable.hpp
        src/scope_guard.hpp
        src/speed_sampler.cpp
        src/speed_sampler.hpp
        src/stream_loader.cpp
        src/stream_loader.hpp
        src/string_utils.cpp
        src/string_utils.hpp
)

# Remove "lib" prefix for dll filename
if(WIN32)
    set_target_properties(BonDriver_EPGStation PROPERTIES PREFIX "")
endif()

# On Windows, Set Character set (TCHAR) to Unicode
if(WIN32)
    target_compile_definitions(BonDriver_EPGStation
        PRIVATE
            UNICODE
            _UNICODE
    )
endif()

# Platform-specific compiler macro / flags
if(MSVC)
    # For MSVC, static link runtime library (MultiThreaded)
    if(${CMAKE_VERSION} VERSION_LESS "3.15")
        target_compile_options(BonDriver_EPGStation
            PUBLIC
                $<$<CONFIG:Debug>:-MTd>
                $<$<CONFIG:Release>:-MT>
                $<$<CONFIG:RelWithDebInfo>:-MT>
                $<$<CONFIG:MinSizeRel>:-MT>
        )
    else()
        set_property(
            TARGET
                BonDriver_EPGStation
            PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
    endif()

    # For MSVC, disable CRT secure warnings
    target_compile_definitions(BonDriver_EPGStation
        PRIVATE
            _CRT_SECURE_NO_WARNINGS=1
    )
elseif(MINGW)
    # For MinGW, static linking runtime library
    target_compile_options(BonDriver_EPGStation
        PUBLIC
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
        ${CPR_INCLUDE_DIRS}
        ${JSON_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link directories
#target_link_directories(BonDriver_EPGStation
#    PRIVATE
#)

# Linking
target_link_libraries(BonDriver_EPGStation
    PRIVATE
        ${CPR_LIBRARIES}
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
            Winmm
            Version
            SetupAPI
            Ws2_32
    )
endif()

if(BONDRIVER_EPGSTATION_BUILD_TEST)
    add_subdirectory(test)
endif()
