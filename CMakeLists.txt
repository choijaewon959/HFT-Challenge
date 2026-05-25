cmake_minimum_required(VERSION 3.16)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)

project(hftproject2026 LANGUAGES CXX)

cmake_policy(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_OPENMP "Enable OpenMP acceleration" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(APPLE)
  execute_process(
          COMMAND xcrun --sdk macosx --show-sdk-path
          OUTPUT_VARIABLE _sdk_path
          OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(_sdk_path)
    message(STATUS "Using macOS SDK: ${_sdk_path}")
    set(CMAKE_OSX_SYSROOT "${_sdk_path}" CACHE PATH "macOS SDK" FORCE)
  endif()
endif()

include(FetchContent)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

add_subdirectory(hftserver2026)
add_subdirectory(hftclient2026)
add_subdirectory(tools)

add_custom_target(all_bins DEPENDS hftserver2026 hftclient2026)