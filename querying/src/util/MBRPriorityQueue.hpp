/**
 * @file
 * A basic Priority Queue (MIN) to keep the results
 */

#ifndef MBR_PRIORITY_QUEUE
#define MBR_PRIORITY_QUEUE

#include <queue>

#include "commons.hpp"
#include "../rtree/RTree.h"

namespace sstss
{
    struct entryIsGreater
    {
        template < typename T >
        bool operator() ( T const l, T const r ) const
        {
            return l.second > r.second;
        }
    };

    struct entryIsLess
    {
        template < typename T >
        bool operator() ( T const l, T const r ) const
        {
            return l.second < r.second;
        }
    };

    class MBRPriorityQueue
    {
        using PQEntry = std::pair< MyTree::Branch*, double >;
        using Q = std::priority_queue< PQEntry, std::vector< PQEntry >, entryIsGreater>;

        Q q; /**< the candidates priority queue */

    public:
        MBRPriorityQueue() {} /**< Empty constructor */
        ~MBRPriorityQueue() {} /**< Empty destructor */

        void add_to_queue(MyTree::Branch* const& branch, double score)
        {
            q.push({branch, score});
        }

        PQEntry toppop()
        {
            PQEntry entry = q.top();
            q.pop();
            return entry;
        }

        double peek () const
        {
            return q.top().second;
        }

        void print()
        {
            while (!q.empty())
            {
                std::cout << "id = " << q.top().first->id << "  score = " << q.top().second << std::endl;
                q.pop();
            }
        }

        bool isEmpty() const
        {
            return q.empty();
        }

        size_t size() const
        {
            return q.size();
        }

    };

} // namespace sstss

#endif
