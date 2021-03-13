cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(BonDriver_EPGStation_test
    EXCLUDE_FROM_ALL
        main.cpp
)

set_target_properties(BonDriver_EPGStation_test
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Platform-specific compiler macro / flags
if(MSVC)
    # For MSVC, disable CRT secure warnings
    target_compile_definitions(BonDriver_EPGStation_test
        PRIVATE
            _CRT_SECURE_NO_WARNINGS=1
    )
elseif(MINGW)
    # For MinGW, static linking runtime library
    target_compile_options(BonDriver_EPGStation_test
        PRIVATE
            -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive -Wl,-Bdynamic
    )
endif()

if(WIN32)
    target_compile_definitions(BonDriver_EPGStation_test
        PRIVATE
            UNICODE
            _UNICODE
    )
endif()

target_include_directories(BonDriver_EPGStation_test
    PRIVATE
        ../include
)

target_link_libraries(BonDriver_EPGStation_test
    PRIVATE
        BonDriver_EPGStation
)
