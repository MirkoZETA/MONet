# OneBigHeader.cmake - Generates a single amalgamated header file
# Automatically resolves dependencies by analyzing #include statements
# Includes both .hpp and .cpp files for a standalone single-header library

set(HEADER_NAME "simulator.hpp")
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME})

# Get all header AND source files
FILE(GLOB_RECURSE all_headers "${PROJECT_SOURCE_DIR}/src/*.hpp")
FILE(GLOB_RECURSE all_sources "${PROJECT_SOURCE_DIR}/src/*.cpp")
STRING(LENGTH "${PROJECT_SOURCE_DIR}/src/" src_prefix_len)

# Build a map of header -> dependencies (normalized paths)
set(approved_headers "")

# We need multiple passes to resolve all dependencies
list(LENGTH all_headers num_headers)
math(EXPR max_iterations "${num_headers} * ${num_headers}")

foreach(iteration RANGE ${max_iterations})
    set(made_progress FALSE)
    
    foreach(header_path ${all_headers})
        # Get relative path from src/
        string(SUBSTRING "${header_path}" ${src_prefix_len} -1 rel_path)
        
        # Skip if already approved
        list(FIND approved_headers "${rel_path}" already_approved)
        if(NOT (already_approved EQUAL -1))
            continue()
        endif()
        
        # Read file content
        file(STRINGS "${header_path}" file_lines)
        
        # Check all dependencies
        set(deps_satisfied TRUE)
        foreach(line ${file_lines})
            # Match #include "something"
            if(line MATCHES "^#include \"([^\"]+)\"")
                set(inc_path "${CMAKE_MATCH_1}")
                
                # Skip system headers and json.hpp (it's self-contained)
                if(inc_path MATCHES "^<" OR inc_path MATCHES "json\\.hpp")
                    continue()
                endif()
                
                # Get directory of current file for resolving relative paths
                get_filename_component(header_dir "${rel_path}" DIRECTORY)
                
                # Resolve the include path relative to current file's directory
                if(inc_path MATCHES "^\\.\\./")
                    # Handle ../ by going up from current directory
                    set(resolved_path "${header_dir}/${inc_path}")
                    # Normalize the path (resolve ..)
                    get_filename_component(resolved_path "${PROJECT_SOURCE_DIR}/src/${resolved_path}" ABSOLUTE)
                    string(SUBSTRING "${resolved_path}" ${src_prefix_len} -1 resolved_rel)
                else()
                    # Direct include from same directory or absolute from src
                    if(EXISTS "${PROJECT_SOURCE_DIR}/src/${header_dir}/${inc_path}")
                        set(resolved_rel "${header_dir}/${inc_path}")
                    else()
                        set(resolved_rel "${inc_path}")
                    endif()
                endif()
                
                # Check if this dependency is already approved
                list(FIND approved_headers "${resolved_rel}" dep_approved)
                if(dep_approved EQUAL -1)
                    set(deps_satisfied FALSE)
                    break()
                endif()
            endif()
        endforeach()
        
        # If all dependencies satisfied, approve this header
        if(deps_satisfied)
            list(APPEND approved_headers "${rel_path}")
            set(made_progress TRUE)
        endif()
    endforeach()
    
    # Check if we're done
    list(LENGTH approved_headers num_approved)
    if(num_approved EQUAL num_headers)
        break()
    endif()
    
    # If no progress made, we have circular dependencies - break and add remaining
    if(NOT made_progress)
        foreach(header_path ${all_headers})
            string(SUBSTRING "${header_path}" ${src_prefix_len} -1 rel_path)
            list(FIND approved_headers "${rel_path}" already_approved)
            if(already_approved EQUAL -1)
                message(WARNING "Could not resolve dependencies for: ${rel_path}, adding anyway")
                list(APPEND approved_headers "${rel_path}")
            endif()
        endforeach()
        break()
    endif()
endforeach()

# Concatenate all headers in resolved order
foreach(header ${approved_headers})
    set(header_path "${PROJECT_SOURCE_DIR}/src/${header}")
    if(EXISTS "${header_path}")
        file(READ "${header_path}" aux)
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME} "${aux}\n")
        message(STATUS "Added header: ${header}")
    endif()
endforeach()

# Now add all .cpp source files (implementations)
foreach(source_path ${all_sources})
    string(SUBSTRING "${source_path}" ${src_prefix_len} -1 rel_path)
    file(READ "${source_path}" aux)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME} "${aux}\n")
    message(STATUS "Added source: ${rel_path}")
endforeach()

# Comment out all internal #include statements
file(READ ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME} filedata)
string(REGEX REPLACE "#include \"" "// #include \"" filedata "${filedata}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME} "${filedata}")

message(STATUS "Generated single header: ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME}")