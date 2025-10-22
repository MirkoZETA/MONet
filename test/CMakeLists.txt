# Prepare "Catch" library for other executables
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
set(CATCH_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# Make test executable
add_custom_target(ALL_TESTS ALL)
include(Catch)
file( GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/*.cpp )
foreach( testsourcefile ${TEST_SOURCES} )
    get_filename_component(filename ${testsourcefile} NAME)
    string( REPLACE ".cpp" "" testname ${filename} )
    add_executable( ${testname} ${testsourcefile} )
    target_link_libraries( ${testname} Catch fnsim )
    catch_discover_tests(${testname})
    add_dependencies(ALL_TESTS ${testname})
endforeach( testsourcefile ${TEST_SOURCES} )