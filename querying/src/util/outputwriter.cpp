/**
 * @file
 * Implementation of output handling methods.
 */

#include "outputwriter.hpp"
#include "constants.hpp"

#include <fstream>

namespace sstss
{
    int OutputWriter::writeResults(Stats const& stats)
    {
        std::ifstream in_file(Constants::RESULT_FILE);
        bool print_header = !in_file.good();
        std::ofstream result_file;
        result_file.open( Constants::RESULT_FILE, std::ofstream::app); // append the current result file

        if (!result_file)
        {
            std::cout << "Couldn't use the output file " << Constants::RESULT_FILE << std::endl;
            return 0;
        }

        if (print_header) { result_file << tag_headers << std::endl; }
        result_file << stats << std::endl;
        return 1;
    }

} // namespace sstss
