cmake_minimum_required(VERSION 3.1)
project(numlin-sat CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(numlin-sat ${PROJECT_SOURCE_DIR}/glucose/core/Solver.cc ${PROJECT_SOURCE_DIR}/glucose/utils/Options.cc ${PROJECT_SOURCE_DIR}/src/Propagator.cc ${PROJECT_SOURCE_DIR}/src/Main.cc ${PROJECT_SOURCE_DIR}/src/Solver.cc)
target_include_directories(numlin-sat PUBLIC ${PROJECT_SOURCE_DIR}/glucose)
