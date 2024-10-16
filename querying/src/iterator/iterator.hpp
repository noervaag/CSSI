//
// Created by silviucm on 10/10/24.
//

#ifndef ITERATOR_HPP
#define ITERATOR_HPP
#include <memory>
#include <queue>
#include "../baseline/baseline.hpp"
#include <faiss/Index.h>

namespace sstss{
    using DocId = uint32_t;
    using resultNext = faiss::idx_t;


class iterator {

  public:
    iterator(faiss::Index* index ,const float* queryVec, size_t dim,  uint32_t k);

    ~iterator();

    bool hasNext();

    resultNext next();

    private:
    faiss::Index* index;
    double lowerbound;
    faiss::idx_t* indices_;
    size_t K_;
    size_t current_index_;
    std::priority_queue<std::pair<double, DocId>> pq;
    const float* query_vec;
    size_t dim;
    size_t k_;
    float* distances;


};


}
#endif //ITERATOR_HPP
