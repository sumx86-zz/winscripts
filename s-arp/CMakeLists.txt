cmake_minimum_required(VERSION 3.10)

# set the project name
project(s-arp VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src/include)

add_definitions(-DDEBUG=1) # comment this line if you dont want the DEBUG option enabled

# add the executable
add_executable(${PROJECT_NAME} src/s-arp.cpp src/util.cpp src/include/s-arp.h src/include/util.h)

target_link_libraries(${PROJECT_NAME} iphlpapi wsock32)