/**
 * @file
 * Definitions of constants.
 */

#ifndef CONSTANTS
#define CONSTANTS

namespace sstss
{
    namespace Constants
    {
        std::string const RESULT_FILE = "../results_log.txt"; /**< The result output file */
        int const RTREEMAXNODES = 64;
	int const NumSpaceDims = 2;

	// The following two are used to avoid resizing vectors
	int const DEFAULTDIM = 100;
	int const DEFAULTLDDIM = 2;
	int const DEFAULTCORPSIZE = 20000000;
    } // namespace Constants
} // namespace sstss

#endif
