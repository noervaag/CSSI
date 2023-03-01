/**
 * @file
 * 
 */

#ifndef SSTSS_RTREEALG
#define SSTSS_RTREEALG

#include "../algorithm/algorithm.hpp"
#include "../util/constants.hpp"
#include "rtreeindex.hpp"

namespace sstss
{
    class RTreeAlg : public Algorithm
    {
    public:
        RTreeAlg() {} /**< Empty constructor */
        ~RTreeAlg() {} /**< Empty destructor */

        RTreeAlg(Corpus const &corpus): Algorithm(corpus) {}

        void preprocess(uint32_t m, double a) override;
        void query(uint32_t k, DocId const& qid, double const& a, ResultVec &results, 
                uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) override;

        RTreeIndex index;

    };

} // namespace sstss

#endif
