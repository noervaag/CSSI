/**
 * @file
 * Implementation of scan algorithm.
 */

#include "scan.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "../util/topkPriorityQueue.hpp"

namespace sstss
{

    void Scan::query(uint32_t k, DocId const& qid, double const& a, ResultVec &results,
		     uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist)
    {
        TopKPriorityQueue docscore_queue(k);
	Doc q = corpus_.docvec.at(qid);
	for(size_t i=0; i<corpus_.num_docs; i++) {
	    double score = distance(q, corpus_.docvec[i], a, corpus_.max_space_distance, corpus_.max_semantic_distance);
	    numdist++;
	    docscore_queue.add_if_better(i, score);
	}
	
        prunes = 0;
	prunes2 = 0;

	int i=0;
	while (!docscore_queue.isEmpty()) {
	    results.push_back(docscore_queue.toppop());
	    i++;
	}
	
    }

} // namespace sstss
