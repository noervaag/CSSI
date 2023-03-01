/**
 * @file
 * A basic Priority Queue (MAX) to keep the results
 */

#ifndef TOPK_PRIORITY_QUEUE
#define TOPK_PRIORITY_QUEUE

#include <queue>

#include "commons.hpp"

namespace sstss
{
    struct myComp {
	constexpr bool operator()(
				  DocScore const& a,
				  DocScore const& b)
	    const noexcept
	{
	    return a.second < b.second;
	}
    };
    
    class TopKPriorityQueue
    {
	std::priority_queue< DocScore, std::vector < DocScore >, myComp > q;
	size_t k;
	
    public:
        TopKPriorityQueue(size_t kk) {k = kk;} 
        ~TopKPriorityQueue() {} /**< Empty destructor */

	/*
        void add_if_better(DocId const& docid, double score)
        {
	    if (q.size() < k ) {
		q.push({docid, score});
	    }
	    else {
		if (score < q.top().second) {
		    q.pop();
		    q.push({docid, score});
		    std::cout << "NEWK\n" ;
		}
	    }
        }
	*/
	// Return true of new element is better and added
	bool add_if_better(DocId const& docid, double score)
        {
	    if (q.size() < k ) {
		q.push({docid, score});
		return true;
	    }
	    else {
		if (score < q.top().second) {
		    q.pop();
		    q.push({docid, score});
		    return true;
		}
	    }
	    return false;
        }


	void push(DocScore ds)
	{
	    q.push(ds);
	}
	
	DocScore top()
        {
            DocScore entry = q.top();
            return entry;
        }

	double peek() const
        {
            return q.top().second;
        }

  
	size_t size()
        {
            return q.size();
        }

   
        DocScore toppop()
        {
            DocScore entry = q.top();
            q.pop();
            return entry;
        }

        bool isEmpty()
        {
            return q.empty();
        }

    };

} // namespace sstss

#endif
