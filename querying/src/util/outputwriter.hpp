/**
 * @file
 * Classes and methods for handling output files.
 */

#ifndef OUTPUTREADER
#define OUTPUTREADER

#include <string>

#include "commons.hpp"

namespace sstss
{
/**
 * Wrapper class for our file parsing methods.
 */
 class OutputWriter {
 public:
     OutputWriter() {} /**< Empty constructor */
     ~OutputWriter() {} /**< Empty destructor */

     int writeResults(Stats const& stats); /**< Outputs the stats into the result file from constants */
 };


} // namespace sstss

#endif
