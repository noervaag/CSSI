/**
 * @file
 * 
 */

#ifndef SSTSS_FAISSHNSW
#define SSTSS_FAISSHNSW

#include "../algorithm/algorithm.hpp"
#include <faiss/IndexHNSW.h>

namespace sstss
{
    class FaissHNSW : public Algorithm
    {
    public:
	FaissHNSW(Corpus const& corpus);
        ~FaissHNSW();

        void query(uint32_t k, DocId const& q, double const& a, ResultVec &results, 
		   uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) override;
	void preprocess(uint32_t m, double a);

    private:
	float* xb;
	faiss::IndexHNSWFlat *index;
	int d = corpus_.dim;
	int nb = corpus_.num_docs; 
    };

} // namespace sstss

#endif
