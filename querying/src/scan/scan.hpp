/**
 * @file
 * 
 */

#ifndef SSTSS_SCAN
#define SSTSS_SCAN

#include "../algorithm/algorithm.hpp"

namespace sstss
{
    class Scan : public Algorithm
    {
    public:
        Scan() {} /**< Empty constructor */
        ~Scan() {} /**< Empty destructor */

        Scan(Corpus const& corpus): Algorithm(corpus) {}

        void query(uint32_t k, DocId const& q, double const& a, ResultVec &results, 
		   uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) override;
    };

} // namespace sstss

#endif
