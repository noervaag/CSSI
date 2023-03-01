/**
 * @file
 * Definition of abstract base class for the various algorithms.
 */

#ifndef ALGORITHM
#define ALGORITHM

#include <cstdint>
#include <algorithm> // std::for_each()

#include "../util/commons.hpp"
#include "../util/constants.hpp"

namespace sstss
{
    /**
     * A generic abstract class for the definition of the common behaviour of SSTSS query algorithms.
     */
     class Algorithm
     {
     public:
         Algorithm() {}
         virtual ~Algorithm() {}

         Algorithm(Corpus const& corpus): corpus_(corpus) {}

         /**
          * Preprocess (untimed)
          */
	 virtual void preprocess(uint32_t m, double a)
	 {
	     (void) m; // Just to silence "unused parameter"
	     (void) a; // Just to silence "unused parameter"
	 }
	 

         /**
          * The main (timed) work to answer a SSTSS query.
          */
          virtual void query(uint32_t /*k*/, DocId const& /*qid*/, double const& /*a*/,
			     ResultVec& /*results*/, uint32_t &/*prunes*/, uint32_t &/*prunes2*/, uint32_t &/*numdist*/) {}

     protected:
         Corpus corpus_;
     private:
     };

} // namespace sstss

#endif
