/**
 * @file
 * 
 */

#ifndef SSTSS_S2R
#define SSTSS_S2R

#include "../algorithm/algorithm.hpp"
#include "../util/constants.hpp"
#include "../util/commons.hpp"
#include "s2rindex.hpp"

namespace sstss
{
    using PivotVec = std::vector< Embedding >; 

    class S2R : public Algorithm
    {
    public:
        S2R() {} /**< Empty constructor */
        ~S2R() {} /**< Empty destructor */

        S2R(Corpus const &corpus): Algorithm(corpus) {}

        void preprocess(uint32_t m, double a) override;
        void query(uint32_t k, DocId const& qied, double const& a, ResultVec &results, 
		   uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) override;

    protected:
	S2RIndex index;
	DocVec pivdocvec; 
	uint32_t pivot_m = 0;
	void find_pivots(PivotVec &pivotvec);
	void convertToPivotedCoords(const Corpus& corp, PivotVec &pivotvec);
     };

} // namespace sstss

#endif
