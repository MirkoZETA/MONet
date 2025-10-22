if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

cmake_minimum_required(VERSION 3.10)
project(multiperiod-optical-network-planning-simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE fns
     CONFIGURE_DEPENDS
     ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE headers
     CONFIGURE_DEPENDS
     ${PROJECT_SOURCE_DIR}/src/*.hpp)

add_library(fnsim STATIC ${fns})

target_include_directories(fnsim PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/alloc
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/math
    ${PROJECT_SOURCE_DIR}/src/sim
)

INSTALL(TARGETS fnsim DESTINATION lib)
INSTALL(FILES ${headers} DESTINATION include/fnsim)

INCLUDE(OneBigHeader.cmake)

add_subdirectory(test)
