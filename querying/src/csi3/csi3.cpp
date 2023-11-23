/**
 * @file
 * 
 */
#include "csi3.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "../util/topkPriorityQueue.hpp"
#include <unordered_set>

namespace sstss
{
 
    void CSI3::preprocess(uint32_t m)
    {
	(void) m; // Just to silence warning of unused parameter

	std::vector<std::vector<std::pair<double, DocId>>> distclustersspace;
	std::vector<std::vector<std::pair<double, DocId>>> distclusterssemantic;
	
	for (auto c :  corpus_.hybrid_index ) {
	    std::vector<std::pair<double, DocId>> dist_docs_in_cluster_space;
	    for (DocId mm :  c.members ) {
		Point cluster_centroid_space = c.scentroid;
		double dist = space_distance(corpus_.docvec.at(mm).first, cluster_centroid_space)/ corpus_.max_space_distance; 
		dist_docs_in_cluster_space.push_back({dist, mm});
	    }
	    sort(dist_docs_in_cluster_space.rbegin(), dist_docs_in_cluster_space.rend());
	    distclustersspace.push_back(dist_docs_in_cluster_space);
	}

	for (auto c :  corpus_.hybrid_index ) { 
	    std::vector<std::pair<double, DocId>> dist_docs_in_cluster_semantic;
	    for (DocId mm :  c.members ) {
		EmbeddingHD cluster_centroid_semantic = c.tcentroid;
		double dist = semantic_distance(corpus_.docvec.at(mm).second, cluster_centroid_semantic)/ corpus_.max_semantic_distance; 
		dist_docs_in_cluster_semantic.push_back({dist, mm});
	    }
	    sort(dist_docs_in_cluster_semantic.rbegin(), dist_docs_in_cluster_semantic.rend());
	    distclusterssemantic.push_back(dist_docs_in_cluster_semantic);
	}

	std::unordered_set<DocId> seen;
	seen.reserve(corpus_.num_docs);
	for(size_t i=0; i < distclustersspace.size(); i++) {
	    std::vector<line_t> dist_docs_in_cluster;
	    for(size_t j=0; j < distclustersspace.at(i).size(); j++ ) {
		line_t tmpline;
		tmpline.objid = -1;
		tmpline.scores = distclustersspace.at(i).at(j).first ;
		tmpline.scoret = distclusterssemantic.at(i).at(j).first;
		
		if (seen.find(distclustersspace.at(i).at(j).second) == seen.end()) {
		    seen.insert(distclustersspace.at(i).at(j).second);
		    tmpline.objid = distclustersspace.at(i).at(j).second;
		    tmpline.doc = corpus_.docvec[tmpline.objid];
		    dist_docs_in_cluster.push_back(tmpline);
		}
		if (seen.find(distclusterssemantic.at(i).at(j).second) == seen.end()) {
		    seen.insert(distclusterssemantic.at(i).at(j).second);
		    tmpline.objid = distclusterssemantic.at(i).at(j).second;
		    tmpline.doc = corpus_.docvec[tmpline.objid];
		    dist_docs_in_cluster.push_back(tmpline);
		}
	    }
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

	    int accessed=0;
	    for ( auto const &line : distclusters.at(sorted_index.at(i).second) ) {
      		double score = distance(q, line.doc, a, corpus_.max_space_distance, corpus_.max_semantic_distance);
		numdist++;
		accessed++;
		docscore_queue.add_if_better(line.objid, score);

		if (docscore_queue.size() == k) {
		    U = docscore_queue.peek();
		    double score_last_line = a*line.scores + (1.0-a)*line.scoret;
		    if (score_last_line < (dist_q_to_cluster_centroid - U)) {
			prunes2 += distclusters.at(sorted_index.at(i).second).size() - accessed;
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

