//
// Created by silviucm on 10/7/24.
//

#ifndef BASELINE_HPP
#define BASELINE_HPP



#include "../algorithm/algorithm.hpp"

namespace sstss
{
    class baseline : public Algorithm
    {
    public:
        baseline() {} /**< Empty constructor */
        ~baseline() {} /**< Empty destructor */

        baseline(Corpus const& corpus): Algorithm(corpus) {}

        void query(uint32_t k, DocId const& q, double const& a, ResultVec &results,
           uint32_t &prunes, uint32_t &prunes2, uint32_t &numdist) override;
        void preprocess(uint32_t m) override;
    private:
        std::vector<std::vector<line_t>> distclusters;
        double computeSpatialDistance(const DocId& id1, const DocId& id2);
        double computeSemanticDistance(const DocId& id1, const DocId& id2);
        double computeCombinedDistance(double ds, double dt, double lambda);
    };

} // namespace sstss

#endif
