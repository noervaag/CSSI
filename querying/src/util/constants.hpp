/**
 * @file
 * Definitions of constants.
 */

#ifndef CONSTANTS
#define CONSTANTS

namespace sstss
{
#define EMBED_DIM_HD 100
#define EMBED_DIM_LD 2

    namespace Constants
    {
        std::string const RESULT_FILE = "../results_log.txt"; /**< The result output file */
        int const RTREEMAXNODES = 64;
	int const NumSpaceDims = 2;

	// To reduce resizing of vectors
	int const DEFAULTCORPSIZE = 5000001;
    } // namespace Constants
} // namespace sstss

#endif
