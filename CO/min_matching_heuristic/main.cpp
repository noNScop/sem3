#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct EdgeHash
{
    size_t operator()(const std::pair<int, int> &edge) const
    {
        static_assert(sizeof(size_t) == 2 * sizeof(int));
        return (size_t)edge.first << (sizeof(int) * 8) | edge.second;
    }
};

using Node = uint32_t;
using NodeList = std::vector<Node>;
using NodeSet = std::unordered_set<Node>;
using EdgeIdxList = std::vector<int>;
using Edge = std::pair<Node, Node>;
using EdgeList = std::vector<Edge>;
using EdgeSet = std::unordered_set<Edge, EdgeHash>;
using AdjacencyList = std::unordered_map<Node, NodeList>;

constexpr std::chrono::milliseconds TIME_LIMIT{27500};

NodeSet vertex_cover(AdjacencyList const &adjlist, EdgeList const &list)
{
    NodeSet cover;
    EdgeSet covered;
    for (int i = 0; i < list.size(); ++i)
    {
        if (covered.count(list[i]))
        {
            continue;
        }

        // add greater degree node to cover
        Node new_covered =
            adjlist.at(list[i].first).size() > adjlist.at(list[i].second).size() ? list[i].first : list[i].second;

        cover.insert(new_covered);

        // mark all edges connected to new_covered as covered
        for (auto const &node : adjlist.at(new_covered))
        {
            covered.insert({new_covered, node});
            covered.insert({node, new_covered});
        }
    }
    return cover;
}

EdgeSet minimum_maximal_from_cover(EdgeList const &list, AdjacencyList const &adjlist, NodeSet const &cover, int N,
                                   bool crimson_planet)
{
    EdgeSet edges;
    NodeSet visited;
    // after adding an edge, mark adjacent nodes and don't add edges adjacent to them in the first iteration
    std::vector<bool> marked_nodes;
    marked_nodes.resize(N);

    // First iteration - respect markings
    for (int i = 0; i < list.size(); ++i)
    {
        if (crimson_planet && (marked_nodes[list[i].first] || marked_nodes[list[i].second]))
        {
            continue;
        }
        if (visited.count(list[i].first) || visited.count(list[i].second))
        {
            continue;
        }

        // Connect if both nodes are in cover first
        if (cover.count(list[i].first) && cover.count(list[i].second))
        {
            edges.insert(list[i]);
            visited.insert(list[i].first);
            visited.insert(list[i].second);
            for (int node : adjlist.at(list[i].first))
            {
                marked_nodes[node] = true;
            }
            for (int node : adjlist.at(list[i].second))
            {
                marked_nodes[node] = true;
            }
        }
    }

    // Second iteration - add remaining edges from vertex cover
    if (crimson_planet)
    {
        for (int i = 0; i < list.size(); ++i)
        {
            if (visited.count(list[i].first) || visited.count(list[i].second))
            {
                continue;
            }

            // Connect if both nodes are in cover first
            if (cover.count(list[i].first) && cover.count(list[i].second))
            {
                edges.insert(list[i]);
                visited.insert(list[i].first);
                visited.insert(list[i].second);
            }
        }
    }

    // Third iteration - add potential remaining edges
    size_t n_remaining = 0;
    for (int i = 0; i < list.size(); ++i)
    {
        if (visited.count(list[i].first) || visited.count(list[i].second))
        {
            continue;
        }

        edges.insert(list[i]);
        visited.insert(list[i].first);
        visited.insert(list[i].second);
        n_remaining++;
    }

    // std::cerr << "needed to add " << n_remaining << " non-cover edges\n";

    return edges;
}

void print_result(EdgeSet const &edges)
{
    std::cout << edges.size() << std::endl;
    for (auto const &edge : edges)
    {
        std::cout << edge.first << " " << edge.second << std::endl;
    }
}

EdgeSet minimum_maximal_matching(AdjacencyList const &adjacency_list, EdgeList const &connections, int N,
                                 bool crimson_planet = true)
{
    NodeSet cover = vertex_cover(adjacency_list, connections);
    auto edges = minimum_maximal_from_cover(connections, adjacency_list, cover, N, crimson_planet);
    // std::cerr << "min max from vertex cover size = " << edges.size() << std::endl;
    return edges;
}

// Idea - each edge leaves behind a crimson circle after it's chosen, nodes in crimson can't be chosen
// So I mark nodes adjacent to chosen edge and then they can't be chosen, until the third iteration in which
// I ensure the matching is maximal. Second iteration can be thougt as extension of the first one, I am using
// a number of iterations as offset for better exploration
EdgeSet MMM_crimson_planet(AdjacencyList const &adjacency_list, EdgeList &connections, int N, int offset)
{
    std::sort(connections.begin(), connections.end(), [&](auto const &a, auto const &b) {
        return adjacency_list.at(a.first).size() + adjacency_list.at(a.second).size() >
               adjacency_list.at(b.first).size() + adjacency_list.at(b.second).size();
    });

    EdgeSet edges;
    NodeSet visited;
    std::vector<bool> marked_nodes;
    marked_nodes.resize(N);

    for (int i = offset; i < connections.size(); ++i)
    {
        if (marked_nodes[connections[i].first] || marked_nodes[connections[i].second])
        {
            continue;
        }
        if (visited.count(connections[i].first) || visited.count(connections[i].second))
        {
            continue;
        }

        edges.insert(connections[i]);
        visited.insert(connections[i].first);
        visited.insert(connections[i].second);
        for (int node : adjacency_list.at(connections[i].first))
        {
            marked_nodes[node] = true;
        }
        for (int node : adjacency_list.at(connections[i].second))
        {
            marked_nodes[node] = true;
        }
    }

    for (int i = 0; i < offset && i < connections.size(); ++i)
    {
        if (marked_nodes[connections[i].first] || marked_nodes[connections[i].second])
        {
            continue;
        }
        if (visited.count(connections[i].first) || visited.count(connections[i].second))
        {
            continue;
        }

        edges.insert(connections[i]);
        visited.insert(connections[i].first);
        visited.insert(connections[i].second);
        for (int node : adjacency_list.at(connections[i].first))
        {
            marked_nodes[node] = true;
        }
        for (int node : adjacency_list.at(connections[i].second))
        {
            marked_nodes[node] = true;
        }
    }

    for (int i = 0; i < connections.size(); ++i)
    {
        if (visited.count(connections[i].first) || visited.count(connections[i].second))
        {
            continue;
        }

        edges.insert(connections[i]);
        visited.insert(connections[i].first);
        visited.insert(connections[i].second);
    }

    return edges;
}

int main()
{
    auto start_time = std::chrono::high_resolution_clock::now();

    EdgeList connections;
    int N;
    int M;

    std::cin >> N >> M;
    connections.reserve(M);

    AdjacencyList adjacency_list;

    for (int i = 0; i < M; ++i)
    {
        int a, b;
        std::cin >> a >> b;
        connections.push_back({a, b});
        adjacency_list[a].push_back(b);
        adjacency_list[b].push_back(a);
    }

    std::random_device rd;
    std::default_random_engine gen(rd());

    // Sometimes specifically in the first run, crimson_planet heuristic yields worse results
    EdgeSet vertex_cover_solution = minimum_maximal_matching(adjacency_list, connections, N, false);
    EdgeSet best_solution = vertex_cover_solution;

    // run it again with crimson_planet heuristic, as it happens that sometimes the first run with edges ordered
    // naturally is the best one
    vertex_cover_solution = minimum_maximal_matching(adjacency_list, connections, N);
    // std::cerr << "Using result from vertex cover, size=" << vertex_cover_solution.size() << "\n";
    if (vertex_cover_solution.size() < best_solution.size())
    {
        best_solution = vertex_cover_solution;
    }

    vertex_cover_solution = MMM_crimson_planet(adjacency_list, connections, N, 0);
    // std::cerr << "Using result from vertex cover, size=" << vertex_cover_solution.size() << "\n";
    if (vertex_cover_solution.size() < best_solution.size())
    {
        best_solution = vertex_cover_solution;
    }

    // Iterate through vertex cover algorithm with shuffled edges
    int iteration = 1;
    while (std::chrono::high_resolution_clock::now() - start_time < TIME_LIMIT)
    {
        std::shuffle(connections.begin(), connections.end(), gen);
        vertex_cover_solution = minimum_maximal_matching(adjacency_list, connections, N);
        // std::cerr << "Using result from vertex cover, size=" << vertex_cover_solution.size() << "\n";
        if (vertex_cover_solution.size() < best_solution.size())
        {
            best_solution = vertex_cover_solution;
        }

        vertex_cover_solution = MMM_crimson_planet(adjacency_list, connections, N, iteration);
        // std::cerr << "Using result from vertex cover, size=" << vertex_cover_solution.size() << "\n";
        if (vertex_cover_solution.size() < best_solution.size())
        {
            best_solution = vertex_cover_solution;
        }
        ++iteration;
    }

    // std::cerr << "Using result from vertex cover, size=" << best_solution.size() << "\n";
    print_result(best_solution);
}