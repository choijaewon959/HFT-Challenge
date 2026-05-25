cmake_minimum_required(VERSION 3.16)
project(hftclient2026 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_OPENMP "Enable OpenMP acceleration" ON)

add_executable(hftclient2026
        main.cpp
        Parser.h
        Profiler.h
        mm.h
)

target_link_libraries(hftclient2026 PRIVATE nlohmann_json::nlohmann_json)

if(USE_OPENMP)
  if(APPLE)
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "/opt/homebrew/opt/libomp/lib/libomp.dylib")
  endif()

  find_package(OpenMP COMPONENTS CXX)

  if(OpenMP_CXX_FOUND)
    target_link_libraries(hftclient2026 PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(hftclient2026 PRIVATE USE_OPENMP)
    message(STATUS "OpenMP enabled for hftclient2026")
  else()
    message(WARNING "OpenMP requested but not found. Building hftclient2026 without OpenMP.")
  endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  target_compile_options(hftclient2026 PRIVATE -Wall -Wextra -Wpedantic)
endif()