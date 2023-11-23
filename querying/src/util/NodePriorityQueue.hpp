/**
 * @file
 * A basic Priority Queue to keep the results
 */

#ifndef NODE_PRIORITY_QUEUE
#define NODE_PRIORITY_QUEUE

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

    class NodePriorityQueue
    {
        using PQEntry = std::pair< MyTree::Node*, double >;
        using Q = std::priority_queue< PQEntry, std::vector< PQEntry >, entryIsGreater >;

        Q q; /**< the candidates priority queue */

    public:
        NodePriorityQueue() {} /**< Empty constructor */
        ~NodePriorityQueue() {} /**< Empty destructor */

        void add_to_queue(MyTree::Node* const& node, double score)
        {
            q.push({node, score});
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
