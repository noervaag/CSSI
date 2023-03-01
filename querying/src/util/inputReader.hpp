/**
 * @file
 * Classes and methods for handling and parsing input files.
 */

#ifndef INPUTREADER
#define INPUTREADER

#include <string>

#include "commons.hpp"

namespace sstss
{

/**
 * Wrapper class for our file parsing methods.
 */
class InputReader {

public:
    InputReader() {} /**< Empty constructor. */
    ~InputReader() {} /**< Empty destructor. */
    
    int readWorkloadFiles( std::string &workloaddir,  Corpus &corpus );
    int readIdxFiles( std::string &idxdir, Corpus &corpus );
    
};

} // namespace sstss

#endif
