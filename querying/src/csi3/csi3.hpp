/**
 * @file
 * Defining 
 */

#ifndef SSTSS_CSI3
#define SSTSS_CSI3

#include "../algorithm/algorithm.hpp"

namespace sstss
{
    class CSI3 : public Algorithm
    {
    public:
        CSI3() {} /**< Empty constructor */
        ~CSI3() {} /**< Empty destructor */

        CSI3(Corpus const& corpus): Algorithm(corpus) {}

        void query(uint32_t k, DocId const& q, double const& a, ResultVec &results, 
		   uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) override;
	void preprocess(uint32_t m, double a);
    private:
	double lowerbound (Doc q, HybridCluster h);
	std::vector<std::vector<std::pair<double, std::pair<DocId, Doc>>>> distclusters;
    };

} // namespace sstss

#endif
