//
// Created by silviucm on 10/10/24.
//

#include "iterator.hpp"
#include "../baseline/baseline.hpp"
#include <faiss/Index.h>

namespace sstss{




    bool iterator::hasNext(){
      if (current_index_ < K_){
        return true;
      }
      else{
        return false;
      }
      }
      iterator::~iterator() {
        delete[] indices_;
        delete[] distances;
        // If you have any other dynamically allocated members, delete them here.
    }

      resultNext iterator::next(){
        if (!hasNext()){
          throw std::out_of_range("out of range ranger");
        }
        return indices_[current_index_++];
      }

      iterator::iterator(faiss::Index* index ,const float* queryVec, size_t dim,  uint32_t k): index(index),query_vec(queryVec),dim(dim),k_(k){
        current_index_ = 0;
        indices_ = new faiss::idx_t[k_];
        distances = new float[k_];
        index->search(1, query_vec, k_ ,distances,indices_);
      }



}