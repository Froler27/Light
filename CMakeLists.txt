cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Light VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS OFF)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${ROOT_DIR}/bin")
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")
set(THIRD_PARTY_DIR "${ROOT_DIR}/3rdparty")

add_subdirectory(3rdparty)
add_subdirectory(renderer)
add_subdirectory(test)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT test)
endif()