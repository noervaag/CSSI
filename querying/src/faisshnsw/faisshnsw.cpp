/**
 * @file
 * Implementation of 
 */

#include "faisshnsw.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "../util/topkPriorityQueue.hpp"
#include <faiss/IndexHNSW.h>

namespace sstss
{
    using idx_t = faiss::idx_t;

    FaissHNSW::FaissHNSW(Corpus const& corpus): Algorithm(corpus)
    {
	xb = new float[d * nb];
	int num_m = 32; // Heuristic...
	index = new faiss::IndexHNSWFlat(d, num_m);
    }

    FaissHNSW::~FaissHNSW()
     {
	 delete xb;
	 delete index;
     }

    void FaissHNSW::preprocess(uint32_t m, double a)
    {
	(void) m; // Just to silence "unused parameter"
	(void) a; // Just to silence "unused parameter"

	for (int i = 0; i < nb; i++) {
	    for (int j = 0; j < d; j++)
		xb[d * i + j] = corpus_.docvec.at(i).second.at(j);
	}

	index->add(nb, xb); // add vectors to the index
	//	index->train(nb, xb); // add vectors to the index
    }

    void FaissHNSW::query(uint32_t k, DocId const& qid, double const& a, ResultVec &results,
		     uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist)
    {
	(void) a; // Just to silence "unused parameter"

	Doc q = corpus_.docvec.at(qid);
	float* xq = new float[d];
	for (int j = 0; j < d; j++)
            xq[j] = q.second.at(j);

	idx_t *I = new idx_t[k];
        float *D = new float[k];
        index->search(1, xq, k, D, I);
	
        prunes = 0;
	prunes2 = 0;
	numdist = 0;

	// Note that distances are recalculated to get them in double as for the other methods
	for(uint32_t i = k; i-- > 0; )
	{
	    double score = distance(q, corpus_.docvec[I[i]], a, corpus_.max_space_distance, corpus_.max_semantic_distance);
	    results.push_back({I[i], score});
	}
	
        delete[] I;
        delete[] D;
	delete[] xq;
    }

} // namespace sstss
