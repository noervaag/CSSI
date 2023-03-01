/**
 * @file
 * Implementation of methods (primarily stream overloading) for our common
 * data structures.
 */

#include "commons.hpp"

#include <algorithm> // std::for_each()

namespace sstss
{
    std::ostream& operator << (std::ostream &o, const Point &p)
    {
        return o << "(" << p.first << "," << p.second << ")";
    }

    std::ostream& operator << (std::ostream &o, const Embedding &ev) /* < Defining << for SemVec */
    {
	o << "[ ";
	for (auto const& e : ev)
	    { o << e << " "; }
	o << "]";
	return o;
    }

    std::ostream& operator << (std::ostream &o, const Doc &d) /**< Defining << for Doc */
    {
	o << d.first << " " << d.second << std::endl;
	return o;
    }
    
    std::ostream& operator << (std::ostream &o, const DocVec &dv)
    {
        for (size_t i=0; i < dv.size(); i++)
        {
	    o << dv[i] << std::endl;
        }
        return o;
    }

    std::ostream& operator << (std::ostream &o, const Corpus &c)
    {
        o << " Dim. of semantic vectors: " << c.dim << std::endl;
        o << " Max space distance = " << c.max_space_distance << std::endl;
	o << " Max semantic distance = " << c.max_semantic_distance << std::endl;
 	o << " Max semantic distance ld = " << c.max_semantic_distance_red << std::endl;
        o << c.docvec << std::endl;
        return o;
    }

    std::ostream& operator << (std::ostream &o, const ResultVec &res)
    {
        o << "[";
        for (auto const& r : res) {
	    o << "(" << r.first << ", " << r.second  << ")";
	}
	o << "]" <<std::endl;
        return o;
    }

    std::ostream& operator << (std::ostream &o, Stats const& stats)
    {
	double csiLm = (double)stats.csiL/sqrt((double)stats.num_docs*0.01);

        o << stats.workloaddir << "\t" << stats.idxdir << "\t" << stats.algorithm << "\t" << stats.alg_index << "\t"
          << stats.query << "\t" << stats.query_index << "\t"
          << stats.k << "\t" << stats.a  << "\t" << stats.m << "\t" << csiLm << "\t" << stats.microseconds_pre << "\t"
          << stats.microseconds_q << "\t" 
          << stats.microseconds_all << "\t" // << stats.peak_rss
	  << stats.num_docs << "\t"
          << stats.prunes  <<  "\t" << stats.prunes2 << "\t" << stats.numdist << "\t" << stats.error;
	if (stats.query_index == 0)
	    o << "\tAGGR";
        return o;
    }

    std::ostream& operator << (std::ostream &o, Headers const&)
    {
	o << "Workloaddir\tIdxdir\tAlg\tAlgIdx\tQuery\tQ index\tk\ta\tm\tLm\tPreT\tQueryT\tTotalT\t"
	  << "NumDocs\tPrunes\tPrunes2\tNumDist\tError\tAGGR";
        return o;
    }

    std::ostream& operator << (std::ostream &o, HybridCluster const& hybridcluster)
    {
	o << "scentroid = " << hybridcluster.scentroid << " sradius = " << hybridcluster.sradius << std::endl; 
	o << "tcentroid = " << hybridcluster.tcentroid << " tradius = " << hybridcluster.tradius << std::endl;
	o  <<" Members: ";
	for (auto const& m : hybridcluster.members)
	    { o << m << " "; }
	o << "\n";
	return o;
    }

    std::ostream& operator << (std::ostream &o, HybridIndex const& hybridindex)
    {
	o << "#hybdridclusters = " << hybridindex.size() << std::endl;
	for (auto const& c : hybridindex)
	    { o << c << " "; }
	return o;
    }
    
} // namespace sstss
