/**
 * @file
 * 
 */

#ifndef SSTSS_RTREEINDEX
#define SSTSS_RTREEINDEX

#include "../util/constants.hpp"
#include "RTree.h"

namespace sstss
{
    using DataType = int;
    using ElemType = coordinate;
    using MyTree = RTree<DataType, ElemType, Constants::NumSpaceDims, double, Constants::RTREEMAXNODES>;

    class RTreeIndex
    {
    public:
        RTreeIndex() {}
        ~RTreeIndex() {}

        void buildIndex(const Corpus& corp);

	void print() ;

	void query(uint32_t k, Doc const& q, double const& a, Corpus const &corp, ResultVec &results,
		   uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist);

    protected:

        void treeInsert(const ElemType a_min[Constants::NumSpaceDims], const ElemType a_max[Constants::NumSpaceDims], const DataType& a_dataId);

	double scoreMBR( MyTree::Branch* a_branch, Point const q, double const a,  Corpus const &corp);
	
	Point minSpaceDistPoint(MyTree::Branch const& a_branch, Point const& q) const;
 
	void PrintData();
	void PrintNodeData(const MyTree::Node* a_node);
	void PrintNodeData(const MyTree::Node* a_node, Doc q, double a, Corpus const &corp);
	
        MyTree rtree;
    };

} // namespace sstss

#endif
