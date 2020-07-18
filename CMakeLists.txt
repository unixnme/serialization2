cmake_minimum_required(VERSION 3.10)
project(serialization)

set(CMAKE_CXX_STANDARD 11)
find_package(Boost 1.41.0 REQUIRED COMPONENTS
    serialization
)
set(Bitsery_INCLUDE_DIRS "bitsery/include")

add_library(LIBRARY INTERFACE)
target_link_libraries(LIBRARY INTERFACE ${Boost_LIBRARIES})
target_include_directories(LIBRARY INTERFACE ${Boost_INCLUDE_DIRS} ${Bitsery_INCLUDE_DIRS})

add_executable(serialization main.cc)
target_link_libraries(serialization LIBRARY)
