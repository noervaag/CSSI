/**
 * @file
 * 
 */
#include <chrono> // for timing
#include "csi3.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "../util/topkPriorityQueue.hpp"

namespace sstss
{
 
    void CSI3::preprocess(uint32_t m, double a)
    {
	(void) m; // Just to silence "unused parameter"

	for (auto c :  corpus_.hybrid_index ) { 
	    std::vector<std::pair<double, std::pair<DocId, Doc>>> dist_docs_in_cluster;
	    for (DocId m :  c.members ) {
		Doc cluster_centroid = {c.scentroid, c.tcentroid};
		double dist = distance(corpus_.docvec.at(m), cluster_centroid, a, corpus_.max_space_distance, corpus_.max_semantic_distance);
		dist_docs_in_cluster.push_back({dist, {m, corpus_.docvec.at(m)}});
	    }
	    sort(dist_docs_in_cluster.rbegin(), dist_docs_in_cluster.rend());
	    distclusters.push_back(dist_docs_in_cluster);
	}
    }


    void CSI3::query(uint32_t k, DocId const& qid, double const& a, ResultVec &results,
		     uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist)
    {
        TopKPriorityQueue docscore_queue(k);
	Doc q = corpus_.docvec.at(qid);
	double U = DBL_MAX;
	prunes = 0;
	prunes2 = 0;

	std::vector < std::pair < double, size_t > > sorted_index; 
	for (size_t i=0; i < corpus_.hybrid_index.size(); i++) {
	    double d =  distance_doc_cluster(q, corpus_.hybrid_index.at(i), a, corpus_.max_space_distance, corpus_.max_semantic_distance);
	    sorted_index.push_back({d, i});
	}
	sort(sorted_index.begin(), sorted_index.end());

	size_t clusters_accessed = 0;
	for(size_t i = 0; i < sorted_index.size(); i++) {
	    clusters_accessed++;
	    HybridCluster c = corpus_.hybrid_index.at(sorted_index.at(i).second);
	    Doc cluster_centroid = {c.scentroid, c.tcentroid};
	    double dist_q_to_cluster_centroid = distance(q, cluster_centroid, a, corpus_.max_space_distance, corpus_.max_semantic_distance);

	    size_t obj_accessed = 0;
	    for ( auto const &obj : distclusters.at(sorted_index.at(i).second) ) {
		obj_accessed++;
		double score = distance(q, obj.second.second, a, corpus_.max_space_distance, corpus_.max_semantic_distance);
		numdist++;
		docscore_queue.add_if_better(obj.second.first, score);
		if (docscore_queue.size() == k) {
		    U = docscore_queue.peek();
		    if (obj.first < (dist_q_to_cluster_centroid - U)) {
			prunes2 += ( distclusters.at(sorted_index.at(i).second).size() - obj_accessed );
			break;
		    }
		}
	    }
	    if ((i+1) <  sorted_index.size()) // We end up accessing all clusters
		if ( sorted_index.at(i+1).first >= U )
		    break;
	}
	
	while (!docscore_queue.isEmpty()) {
	    results.push_back(docscore_queue.toppop());
	}
	if ( clusters_accessed <  sorted_index.size() ) {
	    for (size_t i = clusters_accessed; i < sorted_index.size(); i++) {
		prunes +=  distclusters.at(sorted_index.at(i).second).size();
	    }
	}
    }
    
} // namespace sstss

