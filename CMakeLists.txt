cmake_minimum_required(VERSION 3.14)
project(OurGit CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Core library ──────────────────────────────────────────────────────────────
file(GLOB CORE_SRCS "core/src/*.cpp")
add_library(ourgit_core ${CORE_SRCS})
target_include_directories(ourgit_core PUBLIC core/include)

# ── CLI executable ────────────────────────────────────────────────────────────
add_executable(ourgit cli/main.cpp)
target_link_libraries(ourgit PRIVATE ourgit_core)
