cmake_minimum_required (VERSION 2.6)
project (IPCam)

set (CMAKE_CXX_STANDARD 11)

if (CMAKE_LIBRARY_PATH)
    link_directories(${CMAKE_LIBRARY_PATH})
endif(CMAKE_LIBRARY_PATH)

include_directories ("${PROJECT_SOURCE_DIR}/libs/libv4l2cpp")
add_subdirectory (libs/libv4l2cpp) 

include_directories ("${PROJECT_SOURCE_DIR}/libs/udpClient")
add_subdirectory (libs/udpClient) 

include_directories ("${PROJECT_SOURCE_DIR}/libs/tcpClient")
add_subdirectory (libs/tcpClient) 

include_directories ("${PROJECT_SOURCE_DIR}/libs/performanceLib")
add_subdirectory (libs/performanceLib) 

include_directories ("${PROJECT_SOURCE_DIR}/libs/recordLib")
add_subdirectory (libs/recordLib) 

include_directories ("${PROJECT_SOURCE_DIR}/libs/udpBroadcast")
add_subdirectory (libs/udpBroadcast) 

include_directories ("${PROJECT_SOURCE_DIR}/libs/tcpServer")
add_subdirectory (libs/tcpServer)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/libs")

set(CMAKE_PREFIX_PATH "config")
find_package(MRAA REQUIRED)
if (MRAA_FOUND)
    include_directories(${Mraa_INCLUDE_DIR})
endif(MRAA_FOUND)  

file(GLOB SOURCES "include/*.h" "src/*.cpp")

add_executable (main ${SOURCES})
target_link_libraries (main libv4l2cpp udpClient tcpClient performanceLib recordLib udpBroadcast tcpServer mraa)

# failover

file(GLOB SOURCES "src/failover/*.cpp")

add_executable (failover ${SOURCES})