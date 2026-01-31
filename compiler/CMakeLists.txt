cmake_minimum_required(VERSION 3.10)
project(GlobProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Debug symbols
add_compile_options(-g)

include_directories(src)
include_directories(utils)

file(GLOB SRC_SRC   "src/*.cpp")
file(GLOB SRC_UTILS "utils/*.cpp")


add_executable(dq-comp ${SRC_SRC} ${SRC_UTILS})

# on exception I want to have a nice backtrace
set_target_properties(dq-comp PROPERTIES ENABLE_EXPORTS ON)

target_link_libraries(dq-comp PRIVATE stdc++exp)
