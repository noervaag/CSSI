/**
 * main.cpp
 */

#include <iostream>
#include <boost/program_options.hpp> // for handling input arguments
#include <boost/exception/diagnostic_information.hpp>

#include <chrono> // for timing
#include <sys/resource.h> // for reading mem usage
#include <fstream> // for ifstream
#include <unordered_set>

#include "util/commons.hpp"
#include "util/inputReader.hpp"
#include "algorithm/algorithm.hpp"
#include "util/outputwriter.hpp"
#include "scan/scan.hpp"
#include "rtree/rtreealg.hpp"
#include "s2r/s2r.hpp"
#include "csi3/csi3.hpp"
#include "csia3/csia3.hpp"

namespace po = boost::program_options;

const char* ARG_HELP = "help";
const char* ARG_K = "k";
const char* ARG_Q = "query";
const char* ARG_WORKLOADDIR = "workloaddir";
const char* ARG_IDXDIR = "idxdir";
const char* ARG_ALGORITHM = "algorithm";
const char* ARG_A = "a";
const char* ARG_M = "m";
const char* ARG_L = "L"; // Not really used here, but convenient to get it into the results file
const char* ARG_RNDQ = "rndq";

namespace
{
    struct Parameters
    {
        uint32_t k;
        std::vector< int > query_points;
        std::string workloaddir;
	std::string idxdir;
        std::stringstream algorithms;
        float a;
	int m;
	int csiL;
    };

    struct Rule {};
    Rule const tag_rule;

    std::ostream& operator << (std::ostream& stream, Rule const&)
    {
        stream << "\033[95m";
        for (int i = 0; i < 195; i++) { stream << "\u2500"; }
        return stream << "\033[00m";
    }

    long double median(std::vector< long double > microseconds)
    {
        const auto size = std::distance(microseconds.begin(), microseconds.end());
        std::nth_element(microseconds.begin(), microseconds.begin() + size / 2, microseconds.end());
        return *std::next(microseconds.begin(), size / 2);
    }

    long double sum(std::vector< long double > const& scores)
    {
        double s = std::accumulate (scores.cbegin(), scores.cend(), 0.0,
                []( auto i, auto p ){ return i + p; });
        return s;
    }
    
    long double avg(std::vector< long double > const& scores)
    {
        double s = std::accumulate (scores.cbegin(), scores.cend(), 0.0,
				    []( auto i, auto p ){ return i + p; });
        return s/scores.size();
    }

} // anonymous namespace

int main( int argc, char** argv ) {

    using namespace sstss;

    Parameters parameters;
    Corpus corpus;
    Algorithm *alg;
    Stats stats;
    OutputWriter outWriter;
    std::vector < ResultVec> resultvecs;
    bool exact = false;
    bool exact_been_run_first = false;
    

    // Parameters not used by all algorithms
    parameters.m = -1;
    parameters.csiL = -1;
    
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
	    (ARG_HELP, "produce help message")
	    (ARG_K, po::value< std::uint32_t >(), "parameter k")
	    (ARG_Q, po::value< std::vector< std::string > >()->multitoken(), "query point(s) as index into datafile, multi token")
	    (ARG_WORKLOADDIR, po::value< std::string >(), "set workload directory")
	    (ARG_IDXDIR, po::value< std::string >(), "set index directory")
	    (ARG_ALGORITHM, po::value< std::string >(),
	     "choose algorithm(s), space separated; choices are:"
	     " scan rtree s2r csi3 csia3")
	    (ARG_A, po::value< float >(), "the parameter alpha (aka known as lambda)")
	    (ARG_M, po::value< int >(), "the parameter m")
	    (ARG_L, po::value< int >(), "#uncombined clusters, here to put it into results file only")
	    (ARG_RNDQ, po::value< uint32_t >(), "number of random query points");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc, po::command_line_style::unix_style ^ po::command_line_style::allow_short), vm);
        po::notify(vm);

        /* Help trumps everything */
        if (vm.count(ARG_HELP))
	{
	    std::cout << desc << std::endl;
	    return 0;
	}
        if (vm.count(ARG_WORKLOADDIR))
	{
	    parameters.workloaddir = vm[ARG_WORKLOADDIR].as< std::string >();
	    stats.workloaddir = parameters.workloaddir;
	}
	else
	{
	    std::cout << "You must specify a workload directory" << std::endl;
	    std::cout << desc << std::endl;
	    return 0;
	}
	InputReader ir;
	if (ir.readWorkloadFiles(parameters.workloaddir, corpus) == 1)
	{
	    return 1;
	}

	if (vm.count(ARG_IDXDIR))
        {
	    parameters.idxdir = vm[ARG_IDXDIR].as< std::string >();
	    stats.idxdir = parameters.idxdir;
	    if(ir.readIdxFiles(parameters.idxdir, corpus) == 1) {
		return 1;
	    }
        }

        if (vm.count(ARG_K))
        {
            parameters.k = vm[ARG_K].as< std::uint32_t >();
	    if ( parameters.k < 1 ) {
		std::cout << "You must specify a k >= 1" << std::endl;
		std::cout << desc << std::endl;
		return 0;
	    }
        }
        else
        {
            std::cout << "You must specify a k" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_Q))
        {
            std::vector< std::string > points = vm[ARG_Q].as< std::vector< std::string > >();
            for( std::string p : points ) {
		parameters.query_points.push_back(std::stoi(p));
	    }
        }
	else if (vm.count(ARG_RNDQ))
	{
	    uint32_t numq = vm[ARG_RNDQ].as< std::uint32_t >();
	    for (uint32_t i=0; i < numq; i++) {
		parameters.query_points.push_back(std::rand() % corpus.num_docs);
	    }
	}
        else
        {
            std::cout << "You must specify at least one query point" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_A))
        {
            parameters.a = vm[ARG_A].as< float >();
	    if ((parameters.a < 0) || (parameters.a > 1.0)) {
		std::cout << "Invalid value for alpha." << std::endl;
		return 0;
	    }
        }
        else
        {
            std::cout << "You must specify a parameter alpha" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_M))
        {
            parameters.m = vm[ARG_M].as< int >();
        }
       if (vm.count(ARG_L))
        {
            parameters.csiL = vm[ARG_L].as< int >();
        }
 
        if (vm.count(ARG_ALGORITHM))
        {
            parameters.algorithms.str(vm[ARG_ALGORITHM].as< std::string >());
            std::string next_algorithm;
	    
            std::cout << tag_headers << std::endl << tag_rule << std::endl;
            while (parameters.algorithms >> next_algorithm)
            {
                uint32_t query_index = 1;
                std::unordered_map< std::string, std::vector< long double > > batches;
                for (auto const& q : parameters.query_points)
                {
                    if (next_algorithm.compare("scan") == 0)
                    {
			alg = new Scan(corpus);
                        stats.algorithm = "scan";
                        stats.alg_index = 0;
			exact = true;
                    }
		    else if (next_algorithm.compare("rtree") == 0)
                    {
                        alg = new RTreeAlg(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 1;
			exact = true;
                    }
		    else if (next_algorithm.compare("s2r") == 0)
                    {
			if ( parameters.m == -1) {
			    std::cout << "You must specify m if you want to run s2r" << std::endl;
			    return 0;
			}
                        alg = new S2R(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 2;
			exact = true;
                    }
		    else if (next_algorithm.compare("csi3") == 0)
                    {
			alg = new CSI3(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 5;
			exact = true;
                    }
 		    else if (next_algorithm.compare("csia3") == 0)
                    {
			alg = new CSIA3(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 8;
			exact = false;
                    }
                    else
                    {
                        std::cout << "Algorithm " << next_algorithm << " unknown." << std::endl;
                        std::cout << desc << std::endl;
                        return 0;
                    }

                    stats.query = q;
                    stats.query_index = query_index;
                    stats.k = parameters.k;
                    stats.a = parameters.a;
                    stats.num_docs = corpus.num_docs;
		    stats.m = parameters.m;
		    stats.csiL = parameters.csiL;
                    ResultVec results;
                    uint32_t prunes = 0;
		    uint32_t prunes2 = 0;
		    uint32_t numdist;
		    
		    uint32_t kk = (parameters.k >= corpus.num_docs) ? corpus.num_docs : parameters.k;
                    auto start_preprocess = std::chrono::high_resolution_clock::now();
		    alg->preprocess(parameters.m, parameters.a);
                    auto const elapsed_preprocess = std::chrono::high_resolution_clock::now() - start_preprocess;

                    auto start_q = std::chrono::high_resolution_clock::now();
		    numdist = 0;
		    alg->query(kk, q, parameters.a, results, prunes, prunes2, numdist);
                    auto const elapsed_q = std::chrono::high_resolution_clock::now() - start_q;

                    long long nanosec_preprocess =
                            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_preprocess).count();
                    long long nanosec_q = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_q).count();
                    stats.microseconds_pre = nanosec_preprocess / 1000;
                    stats.microseconds_q = nanosec_q / 1000;
                    stats.microseconds_all = (nanosec_preprocess + nanosec_q ) / 1000;
                    stats.prunes = prunes;
		    stats.prunes2 = prunes2;
		    stats.numdist = numdist;

		    // Calculate error rate, will be shown as -1 if not exact and no exact to compare against
		    if ( !exact ) {
			if ( exact_been_run_first ) { // Will only be calculated if first algorithm run was an exact one, and compare against that...
			    int error_cnt = 0;
			    std::multiset <double> s;
			    for (auto r : results)
				s.insert(r.second);
			    for (uint32_t i=0; i < resultvecs.at(query_index -1).size(); i++) {
				if (s.find(resultvecs.at(query_index -1).at(i).second) == s.end()) {
				    error_cnt++;
				}
				else {
				    s.erase(s.find(resultvecs.at(query_index -1).at(i).second));
				}
			    }
			    stats.error = (double)error_cnt/results.size();
			}
			else { // Not exact, but nothing to compare agaisnt
			    stats.error = -1;
			}
		    }
		    else // is exact
		    { 
			stats.error = 0.0; // Exact algorithm, don't calculate
		    }
		    
		    if(exact && (resultvecs.size() == 0)) { // After first query of first algorithm, true if that one is exact
			exact_been_run_first = true;
		    }

		    std::cout << stats << std::endl;
		    		    
		    resultvecs.push_back(results);
                    query_index++;
		    
                    // save the results for the batches
                    batches["preprocess"].push_back(stats.microseconds_pre);
                    batches["query"].push_back(stats.microseconds_q);
                    batches["all"].push_back(stats.microseconds_all);
                    batches["score"].push_back(stats.actual_score);
                    batches["prunes"].push_back(stats.prunes);
		    batches["prunes2"].push_back(stats.prunes2);
		    batches["numdist"].push_back(stats.numdist);
		    batches["error"].push_back(stats.error);

                    delete alg;
                }

                // batched stats
                stats.query = 0;
                stats.query_index = 0;
                stats.microseconds_pre = median(batches["preprocess"]);
                stats.microseconds_q = median(batches["query"]);
                stats.microseconds_all = median(batches["all"]);
                stats.actual_score = sum(batches["score"]);
                stats.prunes = median(batches["prunes"]);
		stats.prunes2 = median(batches["prunes2"]);
		stats.numdist = median(batches["numdist"]);
  		stats.error = avg(batches["error"]);
                std::cout << stats << std::endl;
                outWriter.writeResults(stats);
            }
        }
        else
        {
            std::cout << "You must specify at least one algorithm" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
    }
    catch (std::exception const& e)
    {
	std::cerr << "error: " << e.what() << std::endl
		  << boost::current_exception_diagnostic_information();
	return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
        return 1;
    }


    // Final check of results, will only be done if last algorithm run is exact, assuming all are (see also error rate)
    if (exact) {
	size_t num_queries = parameters.query_points.size();
	if ( resultvecs.size() > num_queries) { // Make sure more than one algorithm has been run
	    for (size_t i=0; i < num_queries; i++) {
		for (size_t j=0; j < resultvecs.at(i).size(); j++) {
		    // 		std::cout << "Checking: " << resultvecs.at(i).at(j) << " and " << resultvecs.at(i+num_queries).at(j) << std::endl;
		    if ( (resultvecs.at(i).at(j).first != resultvecs.at(i+num_queries).at(j).first)
			 && (resultvecs.at(i).at(j).second != resultvecs.at(i+num_queries).at(j).second) )
		    {
			std::cerr << "Result sets are different, likely bug in one of the algorithms..."  << std::endl;
		    }
		}
	    }
	    std::cout << "Result set verification performed!\n";  
	}
    }
    // Uncomment if you want to see actual results
    //  std::cout << resultvecs.at(0); // Just for quick check, for ONE query point. Note that results are
    //  std::cout << resultvecs.at(1); // sorted on distance only, so order might be different

    return 0;
}
