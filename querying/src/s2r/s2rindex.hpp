/**
 * @file
 * 
 */

#ifndef SSTSS_S2RINDEX
#define SSTSS_S2RINDEX

#include "../util/constants.hpp"
#include "../util/commons.hpp"
#include "RTree.h"

namespace sstss
{
    using DataType = int;
    using ElemType = coordinate;
    using MyTree = RTree<DataType, ElemType, Constants::NumSpaceDims, double, Constants::RTREEMAXNODES>;

    struct SemRect
    {
	Embedding m_min;                      ///< Min dimensions of bounding box
	Embedding m_max;                      ///< Max dimensions of bounding box
    };

    class S2RIndex
    {
    public:
        S2RIndex() {}
        ~S2RIndex() {}

        void buildIndex(const Corpus& corp, const DocVec& pivdocvec);

	void print() ;

	void query(uint32_t k, DocId const& qid, double const& a, Corpus const &corp, ResultVec &results,
		   uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist, DocVec const &pivdocvec);

    protected:

        void treeInsert(const ElemType a_min[Constants::NumSpaceDims], const ElemType a_max[Constants::NumSpaceDims], const DataType& a_dataId);
	SemRect combineSemRect(const SemRect& rectA, const SemRect& rectB);
	void calcSemCovers(MyTree::Node* a_node,  uint32_t* id, DocVec const &pivdocvec);
	double find_max_psem_distance(const DocVec& dv);
	double scoreMBR( MyTree::Branch* a_branch, Doc const q, double const a,  Corpus const &corp); // , DocVec const &pivdocvec);

	Point minSpaceDistPoint(MyTree::Rect const& rect, Point const& q);
	double minSemDist(SemRect const& semRect, Embedding const& q);
  
	void PrintData();
	void PrintNodeData(const MyTree::Node* a_node);
	void PrintNodeData(const MyTree::Node* a_node, Doc q, double a, Corpus const &corp);

	uint32_t pivot_m = 0;
	double max_psem_distance;
        MyTree rtree;
	std::vector< SemRect > semRects;;
    };

} // namespace sstss

#endif
