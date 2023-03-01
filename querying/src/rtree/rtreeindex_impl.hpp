/**
 * @file
 * 
 */

#ifndef SSTSS_RTREEINDEXIMPL
#define SSTSS_RTREEINDEXIMPL

#include "../util/constants.hpp"
#include "../util/commons.hpp"
#include "RTree.h"
#include "../util/NodePriorityQueue.hpp"
#include "../util/topkPriorityQueue.hpp"

namespace sstss
{
    void RTreeIndex::print() 
    {
	std::cout << "Tree:" << std::endl;
	PrintData();
    }

    void RTreeIndex::buildIndex(const Corpus& corp)
    {
        for(size_t docid=0; docid < corp.docvec.size(); docid++)
        {
	    double m[2];
            m[0] = corp.docvec.at(docid).first.first;
            m[1] = corp.docvec.at(docid).first.second;
            treeInsert(m, m, docid);
      }

    }

    void RTreeIndex::treeInsert(const ElemType a_min[Constants::NumSpaceDims], const ElemType a_max[Constants::NumSpaceDims], const DataType& a_dataId)
    {
        rtree.Insert(a_min, a_max, a_dataId);
    }

    void  RTreeIndex::query(uint32_t k, Doc const& q, double const& a, Corpus const &corp, ResultVec &results, 
			    uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist)
    {
	double lowerbound = 0;

        NodePriorityQueue node_queue;
	TopKPriorityQueue topk_queue(k);
	
        MyTree::Node* root = rtree.GetRoot();
        // open the root node
        for(int index = 0; index < root->m_count; ++index)
        {
            node_queue.add_to_queue(root->m_branch[index].m_child, scoreMBR(&root->m_branch[index], q.first, a, corp) );
	}
        while( !node_queue.isEmpty() )
	{
	    auto const [ node, min_score ] = node_queue.toppop(); // dequeue the best scored element
	    
	    if ( (topk_queue.size() == k) && ( min_score > topk_queue.peek()) )
	    {
		prunes++;
	    }
	    else if( node->IsInternalNode() ) // is internal node
	    {
		// add all children to the queue
		for(int i = 0; i < node->m_count; ++i)
		{
		    node_queue.add_to_queue(node->m_branch[i].m_child, scoreMBR(&node->m_branch[i], q.first, a, corp));
		}
	    }
	    else // is leaf
	    {
		lowerbound = min_score;
		for (int i = 0; i < node->m_count; i++) {
		    DocId const docid( node->m_branch[i].m_data );
		    double score = distance(q, corp.docvec.at(docid), a, corp.max_space_distance, corp.max_semantic_distance);
		    numdist++;
		    topk_queue.add_if_better(docid, score);
		}
	    }
	    if ( ( topk_queue.size() > 0) && (lowerbound >= topk_queue.peek())  ) {
		break;
	    }

        }
	int i=0;
	while (!topk_queue.isEmpty()) {
	    results.push_back(topk_queue.toppop());
	    i++;
	}
    }
	
    double RTreeIndex::scoreMBR( MyTree::Branch* a_branch, Point const q, double const a,  Corpus const &corp)
    {
        Point const mindistpoint = minSpaceDistPoint(*a_branch, q);
        return distance_with_equal_semantics(mindistpoint, q, a, corp.max_space_distance);
    }

    Point RTreeIndex::minSpaceDistPoint(MyTree::Branch const& a_branch, Point const& q) const
    {
        double px = q.first;
        double py = q.second;
        double rx, ry;

        if(px < a_branch.m_rect.m_min[0]) { rx = a_branch.m_rect.m_min[0]; }
        else if(px > a_branch.m_rect.m_max[0]) { rx = a_branch.m_rect.m_max[0]; }
        else { rx = px; }

        if(py < a_branch.m_rect.m_min[1]) { ry = a_branch.m_rect.m_min[1]; }
        else if(py > a_branch.m_rect.m_max[1]) { ry = a_branch.m_rect.m_max[1]; }
        else { ry = py; }

        return std::make_pair(rx, ry);
    }


    void RTreeIndex::PrintData() 
    {
	MyTree::Node* first = rtree.GetRoot();
	PrintNodeData(first);
    }

    void RTreeIndex::PrintNodeData(const MyTree::Node* a_node)  {
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
		}
	    }
	}
    }

    void RTreeIndex::PrintNodeData(const MyTree::Node* a_node, Doc q, double a, Corpus const &corp)  {
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
		    std::cout << "   with score = " <<  distance(q, corp.docvec.at(a_node->m_branch[index].m_data), a, corp.max_space_distance, corp.max_semantic_distance) << std::endl;
		    //		    std::cout << "   with WESscore = " << distance_with_equal_semantics(mindistpoint, q, a, corp.max_space_distance);
		    std::cout << "   with id = " << a_node->m_branch[index].id << std::endl;
		}
	    }
	}
    }
 
} // namespace sstss

#endif
