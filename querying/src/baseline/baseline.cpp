// Created by silviucm on 10/7/24.

#include "baseline.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include <unordered_set>
#include <faiss/IndexFlat.h>
#include <faiss/IndexHNSW.h>
#include <faiss/index_io.h>
#include <faiss/Index.h>
#include "../iterator/iterator.hpp"
#include "../util/topkPriorityQueue2.hpp"
#include "../util/topkPriorityQueue.hpp"



namespace sstss {

    baseline::baseline(Corpus const& corpus) : Algorithm(corpus)
{     // d = dimensions, nb = number of docs , xb = vectors index.
     xb = new float[d * nb];
	int num_m = 32; // Heuristic...
	index_ = new faiss::IndexHNSWFlat(d, num_m);

}  //destructor
   baseline::~baseline(){
     delete[] xb;
     delete index_;
   }

    void baseline::preprocess(uint32_t m) {
        (void) m;
        int numM = 32;

        // Build the embeddings array xb from the corpus documents
        for (int i = 0; i < nb; i++) {
            for (int j = 0; j < d; j++) {
                xb[d * i + j] = corpus_.docvec.at(i).second.at(j);
                //xb[d*i + j] makes it so that each dimension is placed in its corresponding place.
            }
        }
        // Add all embeddings to the FAISS index
        index_->add(nb, xb);
    }

    void baseline::query(uint32_t k, DocId const& q, double const& a, ResultVec &results,
                         uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) {
         
        //Number of distance computations
        numdist = 0;   
        //Number of pruned candidates based on early terminations
        prunes = 0;
        //Additional pruning metric (not used)
        prunes2 = 0;
        
        //get the query document query and its embedding (stored in document.second)
        Doc document = corpus_.docvec.at(q);
        //xq = query vector
        float* xq = new float[corpus_.dim];
        // for each dimension of the document, add it to the query vector
        for (int i = 0; i < corpus_.dim; i++) {
            xq[i] = document.second.at(i);
        }


        // Get the query document from the corpus using its ID
        const Doc& query_doc = corpus_.docvec.at(q);
        const EmbeddingHD& q_embedding = query_doc.second;  // Semantic embedding of the query
        const Point& q_point = query_doc.first;             // Spatial coordinates of the query



        //K = the number of retrieved neighbors
        size_t K = 100;
        // I = index , D = distances, k = number of neighbors, xq  = query vector
        faiss::idx_t *I = new faiss::idx_t[K];
        float *D = new float[K];
        //index document with faiss, **remember to also include spatial coordinates.
        index_->search(1, xq, K, D, I);


        // Priority queue. We are using the heap based.
        sstss::TopKPriorityQueue topKqueue(k);

        for (size_t i = 0; i < K; ++i) {
            //Get the documentID for the candidates 
            DocId doc_id = static_cast<DocId>(I[i]);

            //Retrieve the candidate document from the corpus
            const Doc candidate_doc = corpus_.docvec.at(doc_id);            
            const EmbeddingHD& candidate_embedding = candidate_doc.second; // Candidate's embedding
            const Point& candidate_point = candidate_doc.first;           // Candidate's coordinates

            //Semantic distance between query and document
            double dt = std::sqrt(D[i]);

            // Normalize the semantic distance
            double dt_norm = dt / corpus_.max_semantic_distance;

            // Compute the spatial distance between query and candidate
            double ds = space_distance(q_point, candidate_point);
            // Function space_distance computes Euclidean distance between the points

            // Normalize the spatial distance
            double ds_norm = ds / corpus_.max_space_distance;

            // Compute the combined distance using the lambda parameter 'a'
            double d_combined = a * ds_norm + (1 - a) * dt_norm;

            numdist++; // Increment the number of distance computations

            // Add the candidate to the top-k queue if it's among the best
            topKqueue.add_if_better(doc_id, d_combined);

            // Early termination check to potentially stop processing more candidates
            double dLB = (1 - a) * dt_norm; // Lower bound on the combined distance
            if (topKqueue.size() == k && topKqueue.peek() < dLB) {
                // If the current worst distance in top-k is smaller than the lower bound,
                // then no better candidates can be found; we can terminate early
                prunes++;  // Increment the prune counter
                break;     // Exit the loop to stop processing further candidates
            }
        }

        // Extract the results from the priority queue
        while (!topKqueue.isEmpty()) {
            // Remove the top element (candidate with the highest combined distance)
            DocScore result = topKqueue.toppop();
            results.push_back(result); // Add it to the results vector
        }

        // Reverse the results to have them in ascending order of distance
        std::reverse(results.begin(), results.end());
        // Now, results[0] is the nearest neighbor, results[1] is the next nearest, etc

        // Note that distances are recalculated to get them in double as for the other methods
        //for(uint32_t i = k; i-- > 0; ) {
           // double score = distance(document, corpus_.docvec[I[i]], a, corpus_.max_space_distance, corpus_.max_semantic_distance);
            //results.push_back({I[i], score});
        //}

        delete[] I;
        delete[] D;
        delete[] xq;
    }

} // namespace sstss
