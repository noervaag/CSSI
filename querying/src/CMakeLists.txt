cmake_minimum_required( VERSION 3.4 )
project( ssstss )

# set(CMAKE_CXX_COMPILER /usr/bin/clang++)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra -g" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3 -march=native" )
set( basepath "${CMAKE_SOURCE_DIR}/.." )

# find boost
find_package( Boost 1.71.0 COMPONENTS program_options REQUIRED )

# for my code
add_subdirectory( util )
add_subdirectory( algorithm )
add_subdirectory( scan )
add_subdirectory( rtree )
add_subdirectory( s2r )
add_subdirectory( csi3 )
add_subdirectory( csia3 )

# link everything together
add_executable( sstss main.cpp )
# link boost
target_link_libraries( sstss ${Boost_LIBRARIES} )
# link my code
target_link_libraries( sstss util algorithm scan rtree s2r csi3 csia3 )
