//
// Created by silviucm on 10/7/24.
//

#include "baseline.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "../util/topkPriorityQueue.hpp"
#include <unordered_set>
#include <faiss/IndexFlat.h>
#include <faiss/IndexHNSW.h>
#include <faiss/index_io.h>

/* 1. Choose and Set Up the ANN Algorithm
- Use Faiss to find Approximate Nearest Neighbors (ANN).
- Set up Faiss in your C++ project and treat it as a black box.
- Implement an iterator (`next()`) over the results returned by Faiss. If Faiss doesn't provide this directly, retrieve `K` nearest neighbors and create your own iterator to iterate over these results.
*/

namespace sstss{

	void baseline::preprocess(uint32_t m){
    int d = corpus_.dim;
	int nb = corpus_.num_docs;
          (void) m;
    int numM = 32;
	faiss::IndexHNSWFlat indexHNSW(corpus_.dim, numM);

	auto xb = new float[nb * d];

     for (const auto &c : corpus_.hybrid_index){

       for (DocId mm :  c.members){
         auto docEmbedding = corpus_.docvec.at(mm).second;
         xb.insert({docEmbedding,mm});



}

     }
        
        
	};

	void baseline::query(uint32_t k, DocId const& q, double const& a, ResultVec &results,
		   uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist){
		TopKPriorityQueue docscore_queue(k);
        numdist = 0;
        prunes = 0;
        prunes2 = 0;

	for (auto c: corpus_.hybrid_index){

	}

};
}