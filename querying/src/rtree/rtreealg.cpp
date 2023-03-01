/**
 * @file
 * 
 */

#include "rtreealg.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "rtreeindex_impl.hpp"

namespace sstss
{
    void RTreeAlg::preprocess(uint32_t m, double a)
    {
	(void) m; // Just to silence "unused parameter"
	(void) a; // Just to silence "unused parameter"
    
        index.buildIndex(corpus_);
    }

    void RTreeAlg::query(uint32_t k, DocId const& qid, double const& a,  ResultVec &results,
			 uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist)
    {
        Doc q = corpus_.docvec.at(qid);
	prunes = 0;
	prunes2 = 0;
        index.query(k, q, a, corpus_, results, prunes, prunes2, numdist);

    }


} // namespace sstss
