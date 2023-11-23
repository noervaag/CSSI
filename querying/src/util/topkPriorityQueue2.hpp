/**
 * @file
 * A basic Priority Queue (MAX) to keep the results, using array to make it possible to have random access for CSIA
 */

#ifndef TOPK_PRIORITY_QUEUE2
#define TOPK_PRIORITY_QUEUE2

#include <queue>

#include "commons.hpp"

namespace sstss
{
    
    class TopKPriorityQueue2
    {
	std::vector<DocScore> q;
	size_t k;
	
    public:
        TopKPriorityQueue2(size_t kk) {k = kk;} 
        ~TopKPriorityQueue2() {} /**< Empty destructor */

	DocScore at(size_t i)
	{
	    return q.at(i);
	}

	// Return true of new element is better and added
	bool add_if_better(DocId const& docid, double score)
        {
	    if (q.size() < k ) {
		push({docid, score});
		return true;
	    }
	    else if (score < q.back().second) {
		q.pop_back();
		push({docid, score});
		return true;
	    }
	    return false;
        }


	DocScore top()
        {
            DocScore entry = q.back();
            return entry;
        }

	double peek() const
        {
            return q.back().second;
        }

	void push(DocScore ds)
	{
	    if (q.size() == 0) {
		q.push_back(ds);
	    }
	    else {
		size_t i=0;
		for (i=0; i < q.size(); i++) {
		    if (q.at(i).second > ds.second) {
			q.push_back(q.back());
			for (size_t j=q.size()-1; j > i; j--) {
			    q.at(j) = q.at(j-1);
			}
			q.at(i) = ds;
			break;
		    }
		}
		if (i==q.size()) {
		    q.push_back(ds);
		}
	    }
	}
	size_t size()
        {
            return q.size();
        }

   
        DocScore toppop()
        {
            DocScore entry = q.back();
            q.pop_back();
            return entry;
        }

        bool isEmpty()
        {
            return q.empty();
        }

    };

} // namespace sstss

#endif
