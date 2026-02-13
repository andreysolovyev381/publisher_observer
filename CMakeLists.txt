cmake_minimum_required(VERSION 3.1...4.0)
set (PROJECT_NAME "pb_pattern")

project(${PROJECT_NAME}
        VERSION 0.1
        DESCRIPTION "Modified Publisher Observer Pattern"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)


# If CMake policy CMP0167 exists, enable its NEW behavior.
# CMP0167: Controls how the `target_precompile_headers()` command handles relative paths.
# OLD behavior (before CMP0167 is set to NEW) allows CMake’s built-in FindBoost module logic and legacy path interpretation of headers in target_precompile_headers()
# NEW behavior encourages using the upstream BoostConfig.cmake and tighter rules on how paths (relative vs absolute) are handled in target_precompile_headers()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# If CMake policy CMP0135 exists, enable its NEW behavior.
# CMP0135: Ensures that timestamps of downloaded files are preserved when using file(DOWNLOAD) or ExternalProject.
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


# ---------------- ARCHITECTURE STARTS HERE ----------------
set(TSE_ARCH_X86_64 OFF CACHE BOOL "x86_64" FORCE)
set(TSE_ARCH_ARM64 ON CACHE BOOL "arm64" FORCE)
set(TSE_ARCH_NATIVE OFF CACHE BOOL "native" FORCE)

set(_arch_count 0)
foreach(_arch TSE_ARCH_X86_64 TSE_ARCH_ARM64 TSE_ARCH_NATIVE)
    if(${_arch})
        math(EXPR _arch_count "${_arch_count} + 1")
    endif()
endforeach()


if(_arch_count EQUAL 0)
    message(FATAL_ERROR "No architecture option is enabled. Enable exactly one of: TSE_ARCH_X86_64, TSE_ARCH_ARM64, TSE_ARCH_NATIVE.")
elseif(NOT _arch_count EQUAL 1)
    message(FATAL_ERROR "Multiple architecture options are enabled. Enable exactly one of: TSE_ARCH_X86_64, TSE_ARCH_ARM64, TSE_ARCH_NATIVE.")
endif()

function(set_architecture)
    if(TSE_ARCH_X86_64)
        message(STATUS "Project wide selection: Building for Architecture x86_64")
        add_compile_options(
                -march=x86-64-v2
                -mtune=generic
        )
    elseif(TSE_ARCH_ARM64)
        message(STATUS "Project wide selection: Building for Architecture ARM64")
        add_compile_options(
                -march=armv8-a
                -mtune=native
        )
    elseif(TSE_ARCH_NATIVE)
        message(STATUS "Project wide selection: Building for Architecture Native")
        add_compile_options(-march=native)
    else()
        message(FATAL_ERROR "Project wide selection: Architecture is not selected.")
    endif()
endfunction()

set_architecture()

# ---------------- ARCHITECTURE ENDS HERE ----------------





# ---------------- COMPILE OPTIONS STARTS HERE ----------------
if(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL "")
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _build_type_lower)
    if(_build_type_lower STREQUAL "debug")
        message(STATUS "MAIN: Configuring for Debug build")
        add_compile_options(
                -O0
                -g
                -fconcepts-diagnostics-depth=3
                -fdiagnostics-all-candidates
                -fconstexpr-loop-limit=2147483646
                -fconstexpr-ops-limit=2147483646
        )
    else()
        message(STATUS "MAIN: Configuring for Release build")
        add_compile_options(
                -O3
                -fvisibility=hidden
                #obfuscation
                -Wl,--strip-all
                -fno-rtti
                -fmerge-all-constants
                -fno-ident
        )
    endif()
endif()

if (TSE_ARCH_X86_64)
    #obfuscation, works poorly for mac
    add_compile_options(
            -flto #optimization on linker stage
    )
endif ()

add_compile_options(
        -Wall
        -Werror
        -Wextra
        -Wpedantic
)
# ---------------- COMPILE OPTIONS ENDS HERE ----------------







# ---------------- BOOST STARTS HERE ----------------
message(STATUS "MAIN: Working on including Boost...")

# Define possible hardcoded paths
set(HOMEBREW_BOOST_PATH "/opt/homebrew/Cellar/boost/1.88.0")
set(FALLBACK_BOOST_PATH "/Users/elenasolovyeva/SAN/downloads/boost-1.89.0")

# Try to find Boost system-wide
find_package(Boost)

# Decide BOOST_ROOT path (either from system or fallback)
if(Boost_FOUND)
    message(STATUS "MAIN: FOUND Boost system-wide, checking \"BOOST_ROOT\"")

    if(NOT DEFINED BOOST_ROOT)
        set(BOOST_ROOT "${HOMEBREW_BOOST_PATH}")
        message(STATUS "MAIN: BOOST_ROOT not defined, setting to expected Homebrew destination folder: \"${BOOST_ROOT}\"")
    else()
        message(STATUS "MAIN: BOOST_ROOT already defined as: \"${BOOST_ROOT}\"")
    endif()
else()
    set(BOOST_ROOT "${FALLBACK_BOOST_PATH}")
    message(STATUS "MAIN: Boost not found system-wide. Trying fallback path: \"${BOOST_ROOT}\"")
endif()


if(NOT EXISTS "${BOOST_ROOT}")
    set(BOOST_ROOT "/usr/include/boost")
endif()

# 🔒 Validate that BOOST_ROOT exists
if(NOT EXISTS "${BOOST_ROOT}")
    message(FATAL_ERROR "MAIN: BOOST_ROOT was set to \"${BOOST_ROOT}\" but the path does not exist. Aborting.")
else()
    message(STATUS "MAIN: Folder \"${BOOST_ROOT}\" exists, but it doesn't guarantee it is \"BOOST_ROOT\" folder. However we will try that.")
endif()

# Common Boost configuration
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_RUNTIME ON)
include_directories(${BOOST_ROOT})

#boost linkage
#target_link_directories(${target} PRIVATE ${BOOST_ROOT}/lib")
# ---------------- BOOST ENDS HERE ----------------



# ---------------- TESTS ----------------
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)
# For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

set(EXECUTABLE_NAME observer_tests)

add_executable(${EXECUTABLE_NAME}
        ./tests/main.cpp
        ./tests/observer.cpp
)

target_include_directories(${EXECUTABLE_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/
)

target_link_libraries(${EXECUTABLE_NAME}
        PUBLIC
        GTest::gtest_main
        pthread
)
