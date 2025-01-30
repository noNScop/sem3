#include <algorithm>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/// GRAPH START
constexpr size_t MAX_NODES = 70000;

int const INF = std::numeric_limits<int>::max();

struct Vec2
{
    int x = 0;
    int y = 0;
};

struct Edge
{
    size_t destination;
    short weight;
};

class Graph
{
  public:
    using NodeType = size_t;
    using EdgeType = Edge;

    explicit Graph(size_t node_count)
    {
        m_adjacency_list.resize(node_count);
    }

    void add_edge(NodeType start, NodeType end, short weight)
    {
        assert(start < m_adjacency_list.size());
        m_adjacency_list[start].push_back({end, weight});
    }

    auto const &get_edges(NodeType node) const
    {
        assert(node < m_adjacency_list.size());
        return m_adjacency_list[node];
    }

    template <class It> int calculate_path_cost(It begin, It end) const
    {
        if (begin == end)
        {
            return INF;
        }
        int cost = 0;
        for (auto it = begin; it != end - 1; ++it)
        {
            for (auto const &edge : get_edges(*it))
            {
                if (edge.first == *(it + 1))
                {
                    cost += edge.second;
                    break;
                }
            }
        }
        return cost;
    }

    int calculate_path_cost(std::vector<NodeType> const &nodes) const
    {
        return calculate_path_cost(nodes.begin(), nodes.end());
    }

    size_t node_count() const
    {
        return m_adjacency_list.size();
    }

  private:
    std::vector<std::vector<std::pair<NodeType, short>>> m_adjacency_list;
};

///// GRAPH END

using Node = uint32_t;
using FastBool = uint8_t;

constexpr size_t PREPROCESS = false;

int dijkstra(Graph const &graph, Node start, Node goal)
{
    std::vector<int> costs_forward(graph.node_count(), INF);
    std::vector<int> costs_backward(graph.node_count(), INF);
    costs_forward[start] = 0;
    costs_backward[goal] = 0;

    int best = 1e9;
    using Pair = std::pair<int, Node>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<>> pq_forward;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<>> pq_backward;
    pq_forward.push({0, start});
    pq_backward.push({0, goal});

    std::bitset<MAX_NODES> visitedf;
    std::bitset<MAX_NODES> visitedb;

    while (!pq_forward.empty() && !pq_backward.empty())
    {
        int costf = pq_forward.top().first;
        Node nodef = pq_forward.top().second;
        pq_forward.pop();
        if (!visitedf[nodef])
        {
            visitedf[nodef] = true;

            for (auto const &edge : graph.get_edges(nodef))
            {
                int new_cost = costf + edge.second;
                if (new_cost < costs_forward[edge.first])
                {
                    if (visitedb[edge.first])
                    {
                        best = std::min(new_cost + costs_backward[edge.first], best);
                    }
                    costs_forward[edge.first] = new_cost;
                    pq_forward.push({new_cost, edge.first});
                }
            }
        }

        int costb = pq_backward.top().first;
        Node nodeb = pq_backward.top().second;
        pq_backward.pop();
        if (!visitedb[nodeb])
        {
            visitedb[nodeb] = true;

            for (auto const &edge : graph.get_edges(nodeb))
            {
                int new_cost = costb + edge.second;
                if (new_cost < costs_backward[edge.first])
                {
                    if (visitedf[edge.first])
                    {
                        best = std::min(new_cost + costs_forward[edge.first], best);
                    }
                    costs_backward[edge.first] = new_cost;
                    pq_backward.push({new_cost, edge.first});
                }
            }
        }

        if (costb + costf >= best)
        {
            return best;
        }
    }
    // std::cerr << "no path found" << std::endl;
    return 1e9;
}

struct Query
{
    Node start;
    Node goal;
};

int main()
{
    // auto start_time = std::chrono::high_resolution_clock::now();

    size_t n_nodes, n_edges;
    std::cin >> n_nodes >> n_edges;

    Graph graph(n_nodes);
    // std::cerr << "Graph size = " << n_nodes << " nodes, " << n_edges << " edges\n";

    for (size_t start = 0; start < n_nodes; start++)
    {
        size_t n_neighbors;
        std::cin >> n_neighbors;
        for (size_t j = 0; j < n_neighbors; j++)
        {
            size_t end;
            short weight;
            std::cin >> end >> weight;
            // std::cerr << "adding edge " << start << ".." << end << " w=" << weight << std::endl;
            graph.add_edge(start, end, weight);
            graph.add_edge(end, start, weight);
        }
    }
    // std::cerr << "Min weight = " << min_weight << ", max weight = " << max_weight << "\n";

    size_t n_queries;
    std::cin >> n_queries;

    std::vector<Query> queries;
    queries.reserve(n_queries);
    for (int i = 0; i < n_queries; i++)
    {
        Node start, goal;
        std::cin >> start >> goal;
        queries.push_back({start, goal});
    }

    {
        // auto start_time = std::chrono::high_resolution_clock::now();

        for (Query const &query : queries)
        {
            // auto start_time_d = std::chrono::high_resolution_clock::now();
            auto cost = dijkstra(graph, query.start, query.goal);
            // std::cerr << "Query took " << (std::chrono::high_resolution_clock::now() - start_time_d).count() / 1e6 <<
            // "ms" << std::endl;
            std::cout << cost << "\n";
        }
        // std::cerr << "Querying took " << (std::chrono::high_resolution_clock::now() - start_time).count() / 1e6
        //           << "ms\n";
    }
    std::cout << std::flush;

    return 0;
}