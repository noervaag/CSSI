/**
 * @file
 * Implementation of input handling methods.
 */

#include "inputReader.hpp"
#include "constants.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm> // std::for_each()


namespace sstss
{

/**
 * Assumes space-separated file.
 */
    int InputReader::readWorkloadFiles( std::string &workloaddir, Corpus &corpus ) {
	std::string locfilename = workloaddir + "/b.txt";
	std::string semfilename = workloaddir + "/a_hd.txt";
	std::string ldsemfilename = workloaddir + "/a_ld.txt";
	
    // Set initial capacity to save resizing cost
    corpus.docvec.reserve(Constants::DEFAULTCORPSIZE);
    
    std::ifstream inlocfile( locfilename );
    if( inlocfile ) { // check that file opened correctly.
        std::string line;
        while (std::getline(inlocfile, line))
	{
	    Point point;
	    std::stringstream ss;
	    ss.str(line);
	    std::string xcoord, ycoord;
	    std::getline(ss, xcoord, ' ');
	    std::getline(ss, ycoord, ' ');
	    
	    point.first = std::stod(xcoord);
	    point.second = std::stod(ycoord);
	    Embedding e;
	    Doc d = {point, e};
	    corpus.docvec.push_back(d);
	    
	    corpus.xmin = std::min( corpus.xmin, point.first );
	    corpus.xmax = std::max( corpus.xmax, point.first );
	    corpus.ymin = std::min( corpus.ymin, point.second );
	    corpus.ymax = std::max( corpus.ymax, point.second );
	    corpus.num_docs++;
	}
    	inlocfile.close();
	Point pmin = std::make_pair(corpus.xmin, corpus.ymin);
        Point pmax = std::make_pair(corpus.xmax, corpus.ymax);
        corpus.max_space_distance = space_distance(pmin, pmax);
    }
    else {
	std::cerr << "Could not open locinput file for reading: " << locfilename << std::endl;
	return 1;
    }
    
    uint32_t num_sem = 0;
    corpus.dim = 0;
    
    std::ifstream insemfile( semfilename );
    if( insemfile ) { // check that file opened correctly.
        std::string line;
	while (std::getline(insemfile, line))
	{
	    std::stringstream ss;
	    ss.str(line);
	    std::string elem;
	    corpus.docvec.at(num_sem).second.reserve(Constants::DEFAULTDIM);
	    
	    while (std::getline(ss, elem, ' '))
	    {
		corpus.docvec.at(num_sem).second.push_back(std::stod(elem));
	    }

	    if (corpus.docvec.at(num_sem).second.size() == 0) {
		std::cerr << "Line with zero dims read" << std::endl;
		return 1;
	    }
    	    if ( corpus.dim == 0 ) { // Should only be set first time
		corpus.dim = corpus.docvec.at(num_sem).second.size();
	    }
	    if (corpus.dim != corpus.docvec.at(num_sem).second.size()) {
		std::cerr << "Line with different dim then previous read" << std::endl;
		return 1;
	    }
	    num_sem++;
	}
    	insemfile.close();
	if ( num_sem != corpus.num_docs )
	    {
		std::cerr << "Different #lines in locfile (" << corpus.num_docs << " and semfile(" << num_sem << ")" << std::endl;
		return 1;
	    }

	Embedding min_elem(corpus.dim, DBL_MAX);
	Embedding max_elem(corpus.dim, -DBL_MAX);
	for(Doc const &d : corpus.docvec ) {
	    for(size_t i=0; i < corpus.dim; i++) {
		if ( d.second.at(i) < min_elem.at(i) ) {
		    min_elem.at(i) = d.second.at(i);
		}
		if ( d.second.at(i) > max_elem.at(i) ) {
		    max_elem.at(i) = d.second.at(i);
		}
	    }
	}
	corpus.max_semantic_distance = semantic_distance(min_elem, max_elem);
    }
    else {
	std::cerr << "Could not open seminput file for reading: " << semfilename << std::endl;
	return 1;
    }

    uint32_t num_ldsem = 0;
    corpus.ld_dim = 0;

    std::ifstream inldsemfile( ldsemfilename );
    if( inldsemfile ) { // check that file opened correctly.
        std::string line;
	while (std::getline(inldsemfile, line))
	{
	    std::stringstream ss;
	    ss.str(line);
	    std::string elem;
	    Embedding emb;
	    while (std::getline(ss, elem, ' '))
	    {
		emb.push_back(std::stod(elem));
	    }

	    if (emb.size() == 0) {
		std::cerr << "Line with zero ld-dims read" << std::endl;
		return 1;
	    }
    	    if ( corpus.ld_dim == 0 ) { // Should only be set first time
		corpus.ld_dim = emb.size();
	    }
	    if (corpus.ld_dim != emb.size()) {
		std::cerr << "Line with different ld_dim then previous read" << std::endl;
		return 1;
	    }
	    corpus.emb_ld.push_back(emb);

	    num_ldsem++;
	}
    	inldsemfile.close();
	if ( num_ldsem != corpus.num_docs )
	    {
		std::cerr << "Different #lines in ldsemfile (" << corpus.num_docs << " and ldsemfile(" << num_ldsem << ")" << std::endl;
		return 1;
	    }

	Embedding min_elem(corpus.ld_dim, DBL_MAX);
	Embedding max_elem(corpus.ld_dim, -DBL_MAX);
	for(Embedding const &e : corpus.emb_ld ) {
	    for(size_t i=0; i < corpus.ld_dim; i++) {
		if ( e.at(i) < min_elem.at(i) ) {
		    min_elem.at(i) = e.at(i);
		}
		if ( e.at(i) > max_elem.at(i) ) {
		    max_elem.at(i) = e.at(i);
		}
	    }
	}
	corpus.max_semantic_distance_red = semantic_distance(min_elem, max_elem);
    }
    else {
	std::cerr << "Could not open ldseminput file for reading: " << semfilename << std::endl;
	return 1;
    }

    return 0; // Success!
}
    
    
int InputReader::readIdxFiles( std::string &idxdir, Corpus &corpus ) {
    std::string scentroid_filename = idxdir + "/cb.txt";
    std::string tcentroid_filename = idxdir + "/cao.txt";
    std::string tredcentroid_filename = idxdir + "/ca.txt";
    std::string sradius_filename = idxdir + "/rbs.txt";
    std::string tradius_filename = idxdir + "/raos.txt";
    std::string tredradius_filename = idxdir + "/ras.txt";
    std::string combclusers_filename = idxdir + "/comb.txt";

    std::vector < Point > scentroids;
    std::ifstream scentroidfile( scentroid_filename );
    if( scentroidfile ) { // check that file opened correctly.
        std::string line;
        while (std::getline(scentroidfile, line))
	{
	    Point point;
	    std::stringstream ss;
	    ss.str(line);
	    std::string xcoord, ycoord;
	    std::getline(ss, xcoord, ' ');
	    std::getline(ss, ycoord, ' ');
	    
	    point.first = std::stod(xcoord);
	    point.second = std::stod(ycoord);
	    scentroids.push_back(point);
	}
    	scentroidfile.close();
    }
    else {
	std::cerr << "Could not open scentroidfile\n";
	return 1;
    }

    std::vector < Embedding > tcentroids;
    std::ifstream tcentroidfile( tcentroid_filename );
    if( tcentroidfile ) { // check that file opened correctly.
        std::string line;
	uint32_t redemb_dim = 0;
	
        while (std::getline(tcentroidfile, line))
	{
	    std::stringstream ss;
	    ss.str(line);
	    std::string elem;
	    Embedding emb;
	    
	    while (std::getline(ss, elem, ' '))
	    {
		emb.push_back(std::stod(elem));
	    }
	    
	    if (emb.size() == 0) {
		std::cerr << "Line with zero dims read from tcentroidfile\n";
		return 1;
	    }
    	    if ( redemb_dim == 0 ) { // Should only be set first time
		redemb_dim = emb.size();
	    }
	    if (redemb_dim != emb.size()) {
		std::cerr << "Line with different dim then previous read from tcentroidfile\n";
		return 1;
	    }
	    tcentroids.push_back(emb);
	}
    	tcentroidfile.close();
    }
    else {
	std::cerr << "Could not open tcentroidfile\n";
	return 1;
    }

    std::vector < Embedding > tredcentroids;
    std::ifstream tredcentroidfile( tredcentroid_filename );
    if( tredcentroidfile ) { // check that file opened correctly.
        std::string line;
	uint32_t redemb_dim = 0;
	
        while (std::getline(tredcentroidfile, line))
	{
	    std::stringstream ss;
	    ss.str(line);
	    std::string elem;
	    Embedding emb;
	    
	    while (std::getline(ss, elem, ' '))
	    {
		emb.push_back(std::stod(elem));
	    }
	    
	    if (emb.size() == 0) {
		std::cerr << "Line with zero dims read from tredcentroidfile\n";
		return 1;
	    }
    	    if ( redemb_dim == 0 ) { // Should only be set first time
		redemb_dim = emb.size();
		// Check same dim as in a_ld file
		if (redemb_dim != corpus.ld_dim) {
		    std::cout << "redemb_dim != corpus.ld_dim\n";
		    return 1;
		}
	    }
	    if (redemb_dim != emb.size()) {
		std::cerr << "Line with different dim then previous read from tredcentroidfile\n";
		return 1;
	    }
	    tredcentroids.push_back(emb);
	}
    	tredcentroidfile.close();
	/*
	Embedding min_elem(redemb_dim, DBL_MAX);
	Embedding max_elem(redemb_dim, -DBL_MAX);
	for(Embedding const &e : tredcentroids ) {
	    for(size_t i=0; i < redemb_dim; i++) {
		if ( e.at(i) < min_elem.at(i) ) {
		    min_elem.at(i) = e.at(i);
		}
		if ( e.at(i) > max_elem.at(i) ) {
		    max_elem.at(i) = e.at(i);
		}
	    }
	}
	corpus.hybrid_index.max_semantic_distance_red = semantic_distance(min_elem, max_elem);
	*/
    }
    else {
	std::cerr << "Could not open tredcentroidfile\n";
	return 1;
    }



    
    std::vector < double > sradii;
    std::ifstream sradiusfile( sradius_filename );
    if( sradiusfile ) { // check that file opened correctly.
        std::string line;
	
        while (std::getline(sradiusfile, line))
	{
	    sradii.push_back(std::stod(line));
	}
    	sradiusfile.close();
    }
    else {
	std::cerr << "Could not open sradiusfile\n";
	return 1;
    }
    
    std::vector < double > tradii;
    std::ifstream tradiusfile( tradius_filename );
    if( tradiusfile ) { // check that file opened correctly.
        std::string line;
	
        while (std::getline(tradiusfile, line))
	{
	    tradii.push_back(std::stod(line));
	}
    	tradiusfile.close();
    }
    else {
	std::cerr << "Could not open tradiusfile\n";
	return 1;
    }
    
    std::vector < double > tredradii;
    std::ifstream tredradiusfile( tredradius_filename );
    if( tredradiusfile ) { // check that file opened correctly.
        std::string line;
	
        while (std::getline(tredradiusfile, line))
	{
	    tredradii.push_back(std::stod(line));
	}
    	tredradiusfile.close();
    }
    else {
	std::cerr << "Could not open tredradiusfile\n";
	return 1;
    }

    std::unordered_map<int, std::vector < DocId > > cluster_members;
    
    std::ifstream combclusterfile( combclusers_filename );
    if( combclusterfile ) { // check that file opened correctly.
        std::string line;
        while (std::getline(combclusterfile, line))
	{
	    std::vector < DocId > clumembers;
	    uint32_t cluid = std::stoi(line);
	    std::getline(combclusterfile, line);
	    int numincluster = std::stoi(line);
	    for (int i=0; i < numincluster ; i++) {
		std::getline(combclusterfile, line);
		clumembers.push_back(std::stod(line)); // A bit dangerous, if the line is not a double that can be represented as integer, undefined behaviour
	    }
	    cluster_members[cluid] = clumembers;
	}
    	combclusterfile.close();
    }
    else {
	std::cerr << "Could not open combclusterfile\n";
	return 1;
    }
    

    for(size_t i=0; i < tradii.size(); i++) {
	HybridCluster h;
	h.scentroid = scentroids.at(i);
	h.sradius = sradii.at(i);
	h.tcentroid = tcentroids.at(i);
	h.tradius = tradii.at(i);
	h.tredcentroid = tredcentroids.at(i);
	h.tredradius = tredradii.at(i);
	h.members = cluster_members.at(i);
	corpus.hybrid_index.push_back(h);
    }
    return 0;
}
    
} // namespace sstss
