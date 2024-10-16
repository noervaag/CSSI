// Created by silviucm on 10/7/24.

#include "baseline.hpp"                 // Include the header file for the baseline class
#include "../util/commons.hpp"          // Include common utilities and definitions
#include "../util/constants.hpp"        // Include constant definitions
#include <unordered_set>                // Include the unordered_set container (not used in this snippet)
#include <faiss/IndexFlat.h>            // Include FAISS flat index header (not used here)
#include <faiss/IndexHNSW.h>            // Include FAISS HNSW index header
#include <faiss/index_io.h>             // Include FAISS index input/output functions
#include <faiss/Index.h>                // Include base FAISS index class
// #include "../iterator/iterator.hpp"  // Commented out: Iterator class is not used
#include "../util/topkPriorityQueue.hpp" // Include the custom top-k priority queue (heap-based)

namespace sstss {                        // Start of the sstss namespace

    // Constructor for the baseline class, initializes the FAISS index and embeddings
    baseline::baseline(Corpus const& corpus) : Algorithm(corpus) {
        // Initialize dimensions and number of documents from the corpus
        d = corpus.dim;                  // Dimensionality of embeddings
        nb = corpus.num_docs;            // Number of documents in the corpus

        // Allocate memory for the embeddings array
        xb = new float[d * nb];          // Flat array to store all embeddings for FAISS

        int num_m = 32;                  // HNSW parameter: number of neighbors (M)
        // Create a new HNSW index with L2 distance metric
        index_ = new faiss::IndexHNSWFlat(d, num_m);
    }

    // Destructor for the baseline class
    baseline::~baseline() {
        delete[] xb;                     // Deallocate memory for embeddings array
        delete index_;                   // Delete the FAISS index to free memory
    }

    // Preprocess method to build the FAISS index
    void baseline::preprocess(uint32_t m) {
        (void) m; // Unused parameter, silence compiler warning

        // Build the embeddings array xb from the corpus documents
        for (int i = 0; i < nb; i++) {
            const EmbeddingHD& emb = corpus_.docvec.at(i).second; // Get embedding of document i
            for (int j = 0; j < d; j++) {
                // Copy each dimension of the embedding to the flat array
                xb[d * i + j] = static_cast<float>(emb.at(j));    // Convert to float if necessary
            }
        }
        // Add all embeddings to the FAISS index
        index_->add(nb, xb);
        // Note: No need to train the index since we're using IndexHNSWFlat (non-learning index)
    }

    // Query method to perform approximate k-NN search with combined spatial and semantic distances
    void baseline::query(uint32_t k, DocId const& q, double const& a, ResultVec &results,
                         uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) {

        // Initialize counters
        numdist = 0;   // Number of distance computations
        prunes = 0;    // Number of pruned candidates based on early termination
        prunes2 = 0;   // Additional pruning metric (not used here)

        // Get the query document from the corpus using its ID
        const Doc& query_doc = corpus_.docvec.at(q);
        const EmbeddingHD& q_embedding = query_doc.second;  // Semantic embedding of the query
        const Point& q_point = query_doc.first;             // Spatial coordinates of the query

        // Prepare the query embedding array for FAISS
        std::vector<float> xq(corpus_.dim); // Create a vector to hold the query embedding
        for (int i = 0; i < corpus_.dim; i++) {
            // Copy each dimension of the query embedding to the vector
            xq[i] = static_cast<float>(q_embedding.at(i));
        }

        // Set the number of approximate nearest neighbors to retrieve from FAISS
        size_t K = 1000; // You can adjust K based on accuracy/performance trade-off

        // Allocate arrays to hold the search results
        faiss::idx_t* I = new faiss::idx_t[K]; // Array to hold indices of nearest neighbors
        float* D = new float[K];               // Array to hold distances to nearest neighbors

        // Perform the ANN search in the semantic space using FAISS
        index_->search(1, xq.data(), K, D, I);
        // Parameters:
        // - 1: number of queries (we have only one query)
        // - xq.data(): pointer to the query embedding
        // - K: number of nearest neighbors to retrieve
        // - D: array to store distances
        // - I: array to store indices of nearest neighbors

        // Initialize the top-k priority queue (max-heap based on combined distance)
        sstss::TopKPriorityQueue topKqueue(k);

        // Iterate over the retrieved candidates
        for (size_t i = 0; i < K; ++i) {
            // Get the document ID of the candidate
            DocId doc_id = static_cast<DocId>(I[i]);

            // Retrieve the candidate document from the corpus
            const Doc& candidate_doc = corpus_.docvec.at(doc_id);
            const EmbeddingHD& candidate_embedding = candidate_doc.second; // Candidate's embedding
            const Point& candidate_point = candidate_doc.first;           // Candidate's coordinates

            // Compute the semantic distance between query and candidate
            // Note: FAISS returns squared L2 distance, so we take the square root
            double dt = std::sqrt(D[i]);

            // Normalize the semantic distance
            double dt_norm = dt / corpus_.max_semantic_distance;

            // Compute the spatial distance between query and candidate
            double ds = space_distance(q_point, candidate_point);
            // Function space_distance computes Euclidean distance between two points

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
        // Now, results[0] is the nearest neighbor, results[1] is the next nearest, etc.

        // Clean up dynamically allocated memory
        delete[] I; // Delete the array of indices
        delete[] D; // Delete the array of distances
    }

} // namespace sstss
