cmake_minimum_required(VERSION 3.5)
project(sstss)

set(CMAKE_CXX_STANDARD 17)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3 -march=native")

# Set the base path (if needed)
set(basepath "${CMAKE_SOURCE_DIR}/..")

# Find Boost
find_package(Boost 1.71.0 COMPONENTS program_options REQUIRED)

# Add subdirectories for your project components
add_subdirectory(util)
add_subdirectory(algorithm)
add_subdirectory(scan)
add_subdirectory(rtree)
add_subdirectory(s2r)
add_subdirectory(csi3)
add_subdirectory(csia3)
add_subdirectory(baseline)
add_subdirectory(faisshnsw)

# Include Faiss headers
include_directories(/usr/local/include)  # Faiss headers installed here

# Find and configure OpenMP
find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Define the executable
add_executable(sstss main.cpp)

# Link the executable with necessary libraries
target_link_libraries(sstss
        ${Boost_LIBRARIES}
        util
        algorithm
        scan
        csi3
        csia3
        s2r
        rtree
        baseline
        faisshnsw
        /usr/local/lib/libfaiss.so  # Link against the installed Faiss shared library
        pthread
        OpenMP::OpenMP_CXX
)
