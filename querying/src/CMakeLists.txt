cmake_minimum_required( VERSION 3.5 )
project( ssstss )

set(CMAKE_CXX_STANDARD 17)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g" )

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3 -march=native " )
set( basepath "${CMAKE_SOURCE_DIR}/.." )

find_package( Boost 1.71.0 COMPONENTS program_options REQUIRED )

add_subdirectory( util )
add_subdirectory( algorithm )
add_subdirectory( scan )
add_subdirectory( rtree )
add_subdirectory( s2r )
add_subdirectory( csi3 )
add_subdirectory( csia3 )


add_executable( sstss main.cpp )

target_link_libraries( sstss ${Boost_LIBRARIES} )
target_link_libraries( sstss util algorithm scan csi3 csia3 s2r rtree )
target_link_libraries( sstss )


