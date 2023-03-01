/**
 * @file
 * Implementation of basic S2R-based algorithm.
 */

#include "s2r.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "s2rindex_impl.hpp"
#include <boost/range/adaptor/indexed.hpp>
#include <cmath>

namespace sstss
{
    
    void S2R::preprocess(uint32_t m, double a)
    {
	(void) a; // Just to silence "unused parameter"
    
	PivotVec pivotvec;

	pivot_m = m;

	find_pivots(pivotvec);

	convertToPivotedCoords(corpus_, pivotvec);
	
        index.buildIndex(corpus_, pivdocvec);
    }

    void S2R::convertToPivotedCoords(const Corpus& corp, PivotVec &pivotvec)
    {
	pivdocvec.reserve(Constants::DEFAULTCORPSIZE);
	for(size_t docid=0; docid < corp.docvec.size(); docid++)
        {
	    Embedding e;
	    for(uint32_t i = 0; i < pivot_m; i++) {
		e.push_back (semantic_distance(corp.docvec.at(docid).second, pivotvec.at(i)));
	    }
	    pivdocvec.push_back( std::make_pair(corp.docvec.at(docid).first, e) );
	}
    }
 
    void S2R::query(uint32_t k, DocId const& qid, double const& a,  ResultVec &results,
			 uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist)
    {
        prunes = 0;
	prunes2 = 0;
	
	index.query(k, qid, a, corpus_, results, prunes, prunes2, numdist, pivdocvec);

    }

    // The method presented in the S2R paper is N^2 is not scalable, here we use Alg 1. from
    // "The omni-family of all-purpose access methods: a simple and effective way..."
    void S2R::find_pivots(PivotVec &pivotvec)
    {
	std::set < DocId > pivots;
	DocId docid = -1;
	
	Doc s = corpus_.docvec.at(corpus_.num_docs/2); // "Random"
	
	double  dist = -1;
	Embedding f1 = s.second;
	double tmpdist = -1;
	using namespace boost::adaptors;
	for (auto const& d : corpus_.docvec | indexed(0)) {
	    tmpdist = semantic_distance(d.value().second, s.second);
	    if ( tmpdist > dist ) {
		f1 = d.value().second;
		dist = tmpdist;
		docid = d.index();
	    }
	}
	pivots.insert(docid);
	dist = -1;
	Embedding f2 = f1;
	for (auto const& d : corpus_.docvec | indexed(0)) {
	    tmpdist = semantic_distance(d.value().second, f1);
	    if ( tmpdist > dist ) {
		f2 = d.value().second;
		dist = tmpdist;
		docid = d.index();
	    }
	}
	pivots.insert(docid);
	double edge = semantic_distance(f1, f2);
	
	while( pivots.size() < pivot_m ) {
	    DocId candidate = -1;
	    double error = DBL_MAX;
	    for (auto const& d : corpus_.docvec | indexed(0))
	    {
		if ( pivots.find(d.index()) == pivots.end() ) { // Not in F
		    double newerror = 0;
		    for (auto const& p : pivots) {
			newerror += fabs ( edge - semantic_distance(corpus_.docvec.at(p).second, d.value().second) ); 
		    }
		    if ( newerror < error ) {
			error = newerror;
			candidate = d.index();
		    }
		}
	    }
	    pivots.insert(candidate);
	}

	for (auto const& p : pivots) {
	    pivotvec.push_back(corpus_.docvec.at(p).second);
	}
    }
    	    
} // namespace sstss
