/**e
 * @file
 * Definitions of common datastructures and functions
 */

#ifndef COMMONS
#define COMMONS

#include <string>
#include <vector>
#include <iostream> // for overloading stream operators
#include <cfloat> // for min and max values
#include <numeric> // std::accumulate
#include <limits> // std::numeric_limits
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp> // for hash of std::pair
#include <cmath>
#include <assert.h>
#include <boost/stacktrace.hpp>

#include "constants.hpp"


namespace sstss
{
    using coordinate = double; 
    using Point = std::pair< coordinate, coordinate >; /**< a point is a pair of <latitude, longitude> */
    using EmbedElem = double; 
    using Embedding = std::vector<EmbedElem>; // Embedding of one object 
    using EmbedVec = std::vector < Embedding >;
    using Doc = std::pair< Point, Embedding >;
    using DocVec = std::vector< Doc >; //
    using DocId = uint32_t;
    using DocScore = std::pair< DocId, double >;
    using ResultVec = std::vector< DocScore >;

    typedef struct HybridCluster {
	Point scentroid;
	double sradius;
	Embedding tcentroid;
	double tradius;
	Embedding tredcentroid;
	double tredradius;
	std::vector < DocId > members;
    } HybridCluster;

    using HybridIndex = std::vector < HybridCluster >;

    typedef struct Corpus
    {
        Corpus() : xmin( std::numeric_limits< coordinate >::max() )
                 , xmax( std::numeric_limits< coordinate >::max() * -1 )
                 , ymin( std::numeric_limits< coordinate >::max() )
                 , ymax( std::numeric_limits< coordinate >::max() * -1 )
                 , max_space_distance( std::numeric_limits< coordinate >::max() * -1 )
		 , max_semantic_distance( std::numeric_limits< coordinate >::max() * -1 )
		 , num_docs(0) {};
	
	DocVec docvec;
	EmbedVec emb_ld;
        coordinate xmin, xmax, ymin, ymax;
        double max_space_distance;
	double max_semantic_distance;
	double max_semantic_distance_red;
	uint32_t dim;
	uint32_t ld_dim;
        uint32_t num_docs;
	HybridIndex hybrid_index;
	HybridIndex hybrid_index_approx;

    } Corpus;

    /**
     * Struct to keep the parameters and the results of each run
     */
    struct Stats
    {
        std::string workloaddir;
	std::string idxdir;
        std::string algorithm;
        uint32_t alg_index;
        int query;
        uint32_t query_index;
        uint32_t k;
        double a;
	int m;
	int csiL;
        long double microseconds_pre;
        long double microseconds_q;
        long double microseconds_all;
        size_t peak_rss;
        uint32_t num_docs;
        long double actual_score;
        uint32_t prunes;
	uint32_t prunes2;
	uint32_t numdist;
	long double error;
    };

    struct Headers {};
    Headers const tag_headers;

    std::ostream& operator << (std::ostream &o, const Point &p); /**< Defining << for Point */
    std::ostream& operator << (std::ostream &o, const Doc &e); /**< Defining << for Doc */
    std::ostream& operator << (std::ostream &o, const DocVec &dv); /**< Defining << for DocVec */
    std::ostream& operator << (std::ostream &o, const Corpus &c); /**< Defining << for Corpus */
    std::ostream& operator << (std::ostream &o, const ResultVec &res); /**< Defining << for ResultVec */
    std::ostream& operator << (std::ostream &o, Stats const& stats); /**< Defining << for Stats */
    std::ostream& operator << (std::ostream &o, Headers const&); /**< Defining << for Headers */
    std::ostream& operator << (std::ostream &o, HybridCluster const& hybridcluster); 
    std::ostream& operator << (std::ostream &o, HybridIndex const& hybridindex); 
 
    coordinate inline space_distance( Point const& q, Point const& p )
    {
        return sqrt( pow( p.first - q.first, 2 ) + pow( p.second - q.second, 2 ) );
    }

    double inline semantic_distance( Embedding const& q, Embedding const& p)
    {
	// assert ( q.size() == p.size() );

	double temp = 0.0;
	for(std::size_t i = 0; i < q.size() ; i++)
	    temp += pow ( q[i] - p[i], 2 ); 
        return sqrt( temp );
    }


    inline double distance( Doc const& q, Doc const& p, double const a_param,
			   double const max_space_distance, double max_semantic_distance )
    {

	// This one commented out is the one used in the S2R paper, based on sigmoid
	//	double norm_space_distance = -1.0 + ( 2.0 / (1.0 + exp(-space_distance(q.first, p.first))) ); 
	double norm_space_distance = space_distance(q.first, p.first)/max_space_distance; 
	double norm_semantic_distance = semantic_distance(q.second, p.second) / max_semantic_distance;
	return a_param*norm_space_distance + (1.0 - a_param) * norm_semantic_distance;
    }

 
    inline double distance_doc_cluster( Doc const& q, HybridCluster const& c, double const a_param,
			   double const max_space_distance, double max_semantic_distance )
    {

	double norm_space_distance_rad = space_distance(q.first, c.scentroid)/max_space_distance - c.sradius;
	if ( norm_space_distance_rad < 0)
	    norm_space_distance_rad = 0;
	double norm_semantic_distance_rad = semantic_distance(q.second, c.tcentroid) / max_semantic_distance - c.tradius;
	if ( norm_semantic_distance_rad < 0)
	    norm_semantic_distance_rad = 0;
	return a_param*norm_space_distance_rad + (1.0 - a_param) * norm_semantic_distance_rad;
    }

    inline double distance_doc_cluster_ld( Point const& qs, Embedding const& qt, HybridCluster const& c, double const a_param,
			   double const max_space_distance, double max_semantic_distance_red )
    {

	double norm_space_distance_rad = space_distance(qs, c.scentroid)/max_space_distance - c.sradius;
	if ( norm_space_distance_rad < 0)
	    norm_space_distance_rad = 0;
	double norm_semantic_distance_rad = semantic_distance(qt, c.tredcentroid) / max_semantic_distance_red - c.tredradius;
	if ( norm_semantic_distance_rad < 0)
	    norm_semantic_distance_rad = 0;
	return a_param*norm_space_distance_rad + (1.0 - a_param) * norm_semantic_distance_rad;
    }


    inline double distance_with_equal_semantics( Point const& q, Point const& p, double const a_param, double const max_space_distance  )
  
    {
	// This is the one used in the S2R paper, based on sigmoid
	//	double norm_space_distance = -1.0 + ( 2.0 / (1.0 + exp(-space_distance(q.first, p.first))) ); 
	double norm_space_distance = space_distance(q, p)/max_space_distance; 
	// Implicit: double min_semantic_distance = 0.0;
        return a_param*norm_space_distance;
    }
} // namespace sstss

#endif

 
