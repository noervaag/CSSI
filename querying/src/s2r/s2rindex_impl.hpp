/**
 * @file
 * 
 */

#ifndef SSTSS_S2RINDEXIMPL
#define SSTSS_S2RINDEXIMPL

#include "../util/constants.hpp"
#include "../util/commons.hpp"
#include "RTree.h"
#include "../util/NodePriorityQueue.hpp"
#include "../util/topkPriorityQueue.hpp"
#include "s2rindex.hpp"

namespace sstss
{
    std::ostream& operator << (std::ostream &o, const SemRect &r)
    {
	o << "SemRect: "<< std::endl;
	for(size_t index = 0; index < r.m_min.size(); ++index)
	    {
		o << r.m_min.at(index) << " " << r.m_max.at(index) << std::endl;
	    }
	return o;
    }
    
  
    void S2RIndex::print() 
    {
	std::cout << "Tree:\n";
	PrintData();
	std::cout << "SemRects:\n";
	for(size_t i=0; i < semRects.size(); i++) {
	    std::cout << i << std::endl;
	    std::cout << semRects.at(i) << std::endl;
	}
	    
    }

    void S2RIndex::buildIndex(const Corpus& corp, const DocVec& pivdocvec)
    {
	(void) corp; // Silence warning
	pivot_m = pivdocvec.at(0).second.size();
	for(size_t docid=0; docid < pivdocvec.size(); docid++)
	{
	    double m[2];
	    m[0] = pivdocvec.at(docid).first.first;
            m[1] = pivdocvec.at(docid).first.second;
            treeInsert(m, m, docid);
	}

	uint32_t id = 0;
	MyTree::Node* root = rtree.GetRoot();
  	calcSemCovers(root, &id, pivdocvec);

	// Turned out, didin't need this...
	//	max_psem_distance = find_max_psem_distance(pivdocvec);
    }

    // Not needed, can be deleted
    /*    double S2RIndex::find_max_psem_distance(const DocVec& dv)
    {
	Embedding min_elem(pivot_m, DBL_MAX);
	Embedding max_elem(pivot_m, -DBL_MAX);
	for(Doc const &d : dv ) {
	    for(size_t i=0; i < pivot_m; i++) {
		if ( d.second[i] < min_elem[i] ) {
		    min_elem[i] = d.second[i];
		}
		if ( d.second[i] > max_elem[i] ) {
		    max_elem[i] = d.second[i];
		}
	    }
	}
	return semantic_distance(min_elem, max_elem);
    }
    */
    
    void S2RIndex::treeInsert(const ElemType a_min[Constants::NumSpaceDims], const ElemType a_max[Constants::NumSpaceDims], const DataType& a_dataId)
    {
        rtree.Insert(a_min, a_max, a_dataId);
    }

   
    SemRect S2RIndex::combineSemRect(const SemRect& rectA, const SemRect& rectB)
    {
	
	SemRect newSemRect;

	for(size_t index = 0; index < pivot_m; ++index)
	    {
		newSemRect.m_min.push_back(std::min(rectA.m_min[index], rectB.m_min[index]));
		newSemRect.m_max.push_back(std::max(rectA.m_max[index], rectB.m_max[index]));
	    }
	
	return newSemRect;
    }

    void S2RIndex::calcSemCovers(MyTree::Node* a_node,  uint32_t* id, DocVec const &pivdocvec)
    {
	ASSERT(a_node);

	if(a_node->IsInternalNode())  // not a leaf node
	{
	    for(int index = 0; index < a_node->m_count; ++index)
	    {
		calcSemCovers(a_node->m_branch[index].m_child, id, pivdocvec);
            }
	    for(int index = 0; index < a_node->m_count; ++index)
	    {
		a_node->m_branch[index].id = *id;
		*id = *id + 1;
		SemRect newSemRect = semRects.at(a_node->m_branch[index].m_child->m_branch[0].id);
		for(int i = 1; i < a_node->m_branch[index].m_child->m_count; i++)
		{
		    newSemRect = combineSemRect(newSemRect, semRects.at(a_node->m_branch[index].m_child->m_branch[i].id));
		}
		semRects.push_back(newSemRect);
	    }
	}
	else // A leaf node
	{
	    for (int index = 0; index < a_node->m_count; ++index)
	    {
		a_node->m_branch[index].id = *id;
		*id = *id + 1;
		SemRect newSemRect;
		newSemRect.m_min = pivdocvec.at(a_node->m_branch[index].m_data).second;
		newSemRect.m_max = newSemRect.m_min;
		semRects.push_back(newSemRect);
	    }
	}
    }

    // This function partly based on code from the S12R authors
    void  S2RIndex::query(uint32_t k, DocId const& qid, double const& a, Corpus const &corp, ResultVec &results, 
			  uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist, DocVec const &pivdocvec)
    {
	NodePriorityQueue node_queue;
	TopKPriorityQueue topk_queue(k);
	double upperBound = DBL_MAX;
	double lowerBound = 0;
	prunes = 0;
	prunes2 = 0;
	Doc pq = pivdocvec.at(qid);
	Doc q = corp.docvec.at(qid);

	MyTree::Node* root = rtree.GetRoot();

	for(int index = 0; index < root->m_count; ++index)
	{
	    double minD = scoreMBR(&root->m_branch[index], pq, a, corp); // , pivdocvec);
	    node_queue.add_to_queue(root->m_branch[index].m_child, minD);
	}
    
	while( !(node_queue.isEmpty()) ) {
	    auto const [ node, min_score ] = node_queue.toppop(); 
	    
	    if( node->m_level == 0 ) // is leaf node
	    {
		lowerBound = min_score;
		for (int i = 0; i < node->m_count; i++) {
		    DocId const docid(node->m_branch[i].m_data);
		    double score = distance(q, corp.docvec.at(docid), a, corp.max_space_distance, corp.max_semantic_distance);
		    numdist++;
		    if (topk_queue.size() >= k && score >= upperBound)
			continue;
		    else if (topk_queue.size() >= k && score <upperBound)
		    {
			topk_queue.add_if_better(docid, score);
			upperBound = topk_queue.peek();
		    }
		    else if (topk_queue.size() < k)
		    {
			if (score > upperBound)
			{
			    upperBound = score;
			    topk_queue.add_if_better(docid, score);
			}
			else
			{
			    topk_queue.add_if_better(docid, score);
			    if (topk_queue.size() == 1)
				upperBound = score;
			}
		    }
		}
	    } else // Is internal node
	    {
		for(int index = 0; index < node->m_count; ++index)
		{
		    double minD = scoreMBR(&node->m_branch[index], pq, a, corp); // , pivdocvec);
		    node_queue.add_to_queue(node->m_branch[index].m_child, minD);
		}
	    }
	    if ( lowerBound >= upperBound ) {
		break;
	    }
	}

	int i=0;
	while (!topk_queue.isEmpty()) {
	    results.push_back(topk_queue.toppop());
	    i++;
	}

    }
    
    double S2RIndex::scoreMBR( MyTree::Branch* a_branch, Doc const q, double const a,  Corpus const &corp) //, const DocVec& pivdocvec)
    {
        Point const minspacedistpoint = minSpaceDistPoint(a_branch->m_rect, q.first);
	double minsemdist =  minSemDist(semRects.at(a_branch->id), q.second);

	double norm_space_distance = space_distance(q.first, minspacedistpoint)/corp.max_space_distance; 
	double norm_semantic_distance = minsemdist / corp.max_semantic_distance;
        return a*norm_space_distance + (1.0 - a) * norm_semantic_distance;

    }

    Point S2RIndex::minSpaceDistPoint(MyTree::Rect const& rect, Point const& q) 
    {
        double px = q.first;
        double py = q.second;
        double rx, ry;

        if(px < rect.m_min[0]) { rx = rect.m_min[0]; }
        else if(px > rect.m_max[0]) { rx = rect.m_max[0]; }
        else { rx = px; }

        if(py < rect.m_min[1]) { ry = rect.m_min[1]; }
        else if(py > rect.m_max[1]) { ry = rect.m_max[1]; }
        else { ry = py; }

        return std::make_pair(rx, ry);
    }

#define rMIN(a, b) ((a) < (b) ? (a) : (b))
#define rMAX(a, b) ((a) > (b) ? (a) : (b))

    // Based on code from the S2R authors
    double S2RIndex::minSemDist(SemRect const& semRect, Embedding const& q) 
    {
	double minTD = -1;

	for (size_t i=0; i < pivot_m; i++) {
	    double min = semRect.m_min.at(i);
	    double max = semRect.m_max.at(i);
	    if (q.at(i) <= min) {
		minTD = rMAX(minTD, min - q.at(i));
	    }
	    else if (q.at(i) > max) {
		minTD = rMAX(minTD, q.at(i) - max);
	    } else {
		minTD = rMAX(minTD, 0);
	    }
	}
        return minTD;
    }
   
    void S2RIndex::PrintData() 
    {
	MyTree::Node* first = rtree.GetRoot();
	PrintNodeData(first);
    }

    void S2RIndex::PrintNodeData(const MyTree::Node* a_node)  {
	if(!(a_node->IsLeaf()))
	{
	    for(int i = 0; i < a_node->m_count; ++i)
	    {
		PrintNodeData(a_node->m_branch[i].m_child);
	    }
	    if (a_node)
	    {
		if (a_node->IsInternalNode()) {
		    std::cout << " - Internal node" << std::endl;
	    } else {
                std::cout << "  - Leaf node" << std::endl;
		}
		std::cout << "   with count = " << a_node->m_count << std::endl;
		std::cout << "   with level = " << a_node->m_level << std::endl;
		for (int index = 0; index < a_node->m_count; ++index) {
		    std::cout << "   with rect min = (" << a_node->m_branch[index].m_rect.m_min[0] << ","
			      << a_node->m_branch[index].m_rect.m_min[1] << ")" << std::endl;
		    std::cout << "   with rect max = (" << a_node->m_branch[index].m_rect.m_max[0] << ","
			      << a_node->m_branch[index].m_rect.m_max[1] << ")" << std::endl;
		    std::cout << "   with id = " << a_node->m_branch[index].id << std::endl;
		    std::cout << "   with semBB = " << semRects.at(a_node->m_branch[index].id) << std::endl;
				    
		}
	    }
	}
	else {
	    if (a_node) {
		if (a_node->IsInternalNode()) {
		    std::cout << " - Internal node" << std::endl;
		} else {
		    std::cout << "  - Leaf node" << std::endl;
		}
		std::cout << "   with count = " << a_node->m_count << std::endl;
		std::cout << "   with level = " << a_node->m_level << std::endl;
		for (int index = 0; index < a_node->m_count; ++index) {
		    std::cout << "   with rect min = (" << a_node->m_branch[index].m_rect.m_min[0] << ","
			      << a_node->m_branch[index].m_rect.m_min[1] << ")" << std::endl;
		    std::cout << "   with rect max = (" << a_node->m_branch[index].m_rect.m_max[0] << ","
			      << a_node->m_branch[index].m_rect.m_max[1] << ")" << std::endl;
		    std::cout << "   with data = " << a_node->m_branch[index].m_data << std::endl;
		    std::cout << "   with id = " << a_node->m_branch[index].id << std::endl;
		    std::cout << "   with semBB = " << semRects.at(a_node->m_branch[index].id) << std::endl;

		}
	    }
	}
    }

    void S2RIndex::PrintNodeData(const MyTree::Node* a_node, Doc q, double a, Corpus const &corp)  {
	if(!(a_node->IsLeaf()))
	{
	    for(int i = 0; i < a_node->m_count; ++i)
	    {
		PrintNodeData(a_node->m_branch[i].m_child, q, a, corp);
	    }
	    if (a_node)
	    {
		if (a_node->IsInternalNode()) {
		    std::cout << " - Internal node" << std::endl;
		} else {
		    std::cout << "  - Leaf node" << std::endl;
		}
		std::cout << "   with count = " << a_node->m_count << std::endl;
		std::cout << "   with level = " << a_node->m_level << std::endl;
		for (int index = 0; index < a_node->m_count; ++index) {
		    std::cout << "   with rect min = (" << a_node->m_branch[index].m_rect.m_min[0] << ","
			      << a_node->m_branch[index].m_rect.m_min[1] << ")" << std::endl;
		    std::cout << "   with rect max = (" << a_node->m_branch[index].m_rect.m_max[0] << ","
			      << a_node->m_branch[index].m_rect.m_max[1] << ")" << std::endl;
		    std::cout << "   with id = " << a_node->m_branch[index].id << std::endl;
		    
		}
	    }
	}
	else {
	    if (a_node) {
		if (a_node->IsInternalNode()) {
		    std::cout << " - Internal node" << std::endl;
		} else {
		    std::cout << "  - Leaf node" << std::endl;
		}
		std::cout << "   with count = " << a_node->m_count << std::endl;
		std::cout << "   with level = " << a_node->m_level << std::endl;
		for (int index = 0; index < a_node->m_count; ++index) {
		    std::cout << "   with rect min = (" << a_node->m_branch[index].m_rect.m_min[0] << ","
			      << a_node->m_branch[index].m_rect.m_min[1] << ")" << std::endl;
		    std::cout << "   with rect max = (" << a_node->m_branch[index].m_rect.m_max[0] << ","
			      << a_node->m_branch[index].m_rect.m_max[1] << ")" << std::endl;
		    std::cout << "   with data = " << a_node->m_branch[index].m_data << std::endl;
		    std::cout << "   with score = " <<  distance(q, corp.docvec.at(a_node->m_branch[index].m_data), a, corp.max_space_distance, max_psem_distance) << std::endl;
		    //		    std::cout << "   with WESscore = " << distance_with_equal_semantics(mindistpoint, q, a, corp.max_Space_distance, max_psem_distance);
		    std::cout << "   with id = " << a_node->m_branch[index].id << std::endl;
		}
	    }
	}
    }
 
} // namespace sstss

#endif
