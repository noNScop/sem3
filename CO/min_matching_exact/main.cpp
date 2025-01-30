#include <algorithm>
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

using Node = short;
using NodeList = std::vector<Node>;
using NodeSet = std::unordered_set<Node>;
using EdgeIdxList = std::vector<short>;
using Edge = std::pair<Node, Node>;
using EdgeList = std::vector<Edge>;
using EdgeSet = std::unordered_set<Edge, EdgeHash>;
using AdjListStatic = std::vector<NodeList>;
using Bool = uint8_t;
using AdjMatrix = std::vector<std::vector<Bool>>;
using AdjList = std::vector<NodeList>;

short N, M;
AdjListStatic FULL_GRAPH;
std::vector<short> DEGREES;
EdgeList MIN_MATCHING;
short MIN_SIZE = std::numeric_limits<short>::max();

AdjMatrix graph;
std::vector<short> degrees;
EdgeList matching;
EdgeList connections;
std::vector<Bool> visited;

void print_result(EdgeList const &edges)
{
    std::cout << edges.size() << std::endl;
    for (auto const &edge : edges)
    {
        std::cout << edge.first << " " << edge.second << std::endl;
    }
}

Bool reduce_graph(EdgeList &removed_edges, EdgeList &add_edges)
{
    // if there are 2 degree-1 nodes connected, add them to the matching,
    EdgeList edge_between_degree1;
    for (Node node = 0; node < N; ++node)
    {
        if (degrees[node] == 1)
        {
            // "v" - node adjacent to degree-1 node "node"
            Node v;
            for (v = 0; v < N; ++v)
            {
                if (graph[node][v])
                {
                    break;
                }
            }

            if (degrees[v] == 1)
            {
                // if v has degree = 1, add it to the matching (it gets added from add_edges in branch_and_reduce())
                add_edges.push_back({std::min(v, node), std::max(v, node)});
                visited[v] = true;
                visited[node] = true;

                graph[node][v] = false;
                graph[v][node] = false;
                removed_edges.push_back({node, v});
                degrees[node] = 0;
                degrees[v] = 0;
            }
            else if (degrees[v] == 2)
            {
                edge_between_degree1.push_back({v, node});
            }
        }
    }

    // If there is a line of 3 edges (u - v - w - x), add the middle edge (v - w) to the matching.
    // This is because adding the middle edge (v - w) covers the central vertices (v and w) with a single edge,
    // which is optimal. Adding both u - v and w - x would be suboptimal as it uses two edges to cover the same vertices.
    // Therefore, adding the middle edge (v - w) simplifies the graph and helps in reducing the search space.
    // If it leads to invalid solution - it means that suboptimal choices has been made in the branch and we can abandon it.
    for (short i = 0; i < edge_between_degree1.size(); ++i)
    {
        for (short j = i + 1; j < edge_between_degree1.size(); ++j)
        {
            Node u = edge_between_degree1[i].first;
            Node v = edge_between_degree1[j].first;
            if (graph[u][v])
            {
                // add u-v to the matching (it gets added from add_edges in branch_and_reduce())
                add_edges.push_back({std::min(u, v), std::max(u, v)});
                visited[u] = true;
                visited[v] = true;

                NodeList degree0({edge_between_degree1[i].second, edge_between_degree1[j].second});

                // Remove u and v
                for (short w = 0; w < N; ++w)
                {
                    if (graph[u][w])
                    {
                        --degrees[w];
                        graph[w][u] = false;
                        graph[u][w] = false;
                        removed_edges.push_back({w, u});
                    }
                    if (graph[v][w])
                    {
                        --degrees[w];
                        graph[w][v] = false;
                        graph[v][w] = false;
                        removed_edges.push_back({w, v});
                    }
                }

                degrees[u] = 0;
                degrees[v] = 0;

                // Check if the current state leads to maximal matching, if not revert graph to the state before invoking this function
                // Check all nodes that degree has gone to 0 after adding an edge
                for (const Node node : degree0)
                {
                    // If originally this node had more than 1 neighbour
                    if (FULL_GRAPH[node].size() > 1 && !visited[node])
                    {
                        // check if all its neighbours are either in the graph or in the matching
                        for (const Node n : FULL_GRAPH[node])
                        {
                            if (!visited[n] && !degrees[n])
                            {
                                // If not, revert graph to its state before invoking this function
                                for (const Edge &edge : removed_edges)
                                {
                                    graph[edge.first][edge.second] = true;
                                    graph[edge.second][edge.first] = true;
                                    visited[edge.first] = false;
                                    visited[edge.second] = false;
                                    ++degrees[edge.first];
                                    ++degrees[edge.second];
                                }
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

void branch_and_reduce(short idx)
{
    EdgeList add_edges;
    EdgeList reduced_edges;
    if (reduce_graph(reduced_edges, add_edges))
    {
        return;
    }
    // update graph size, and matching after reduce_graph()
    M -= reduced_edges.size();
    for (short i = add_edges.size() - 1; i >= 0; --i)
    {
        matching.push_back(add_edges[i]);
    }

    Bool empty = true;
    for (short d : degrees)
    {
        if (d)
        {
            empty = false;
            break;
        }
    }

    if (empty)
    {
        if (matching.size() < MIN_SIZE)
        {
            MIN_SIZE = matching.size();
            MIN_MATCHING = matching;
        }
        // revert graph to its previous state
        for (const Edge &edge : reduced_edges)
        {
            graph[edge.first][edge.second] = true;
            graph[edge.second][edge.first] = true;
            ++degrees[edge.first];
            ++degrees[edge.second];
        }
        for (const Edge &edge : add_edges)
        {
            matching.pop_back();
            visited[edge.first] = false;
            visited[edge.second] = false;
        }
        M += reduced_edges.size();
        return;
    }

    // M is the current number of edges, the loop below finds an edge of highest degree
    short max = 2; // Initialize to a value smaller than any possible degree

    for (short i = idx; i < connections.size(); ++i)
    {
        if (degrees[connections[i].first] && degrees[connections[i].second])
        {
            short d = degrees[connections[i].first] + degrees[connections[i].second];
            if (d > max)
            {
                max = d;
            }
        }
    }
    // -1 because the edge it self is counted twice (a -> b) and (a <- b)
    --max;

    // Branch & Bound: if we have already exceeded the best known
    // solution, don't search this branch further
    // Adding an edge can remove itself and all adjacent edges, this reduces the degree of other edges,
    // so (M / max) is number of edges needed to add to matching if all the added edges would have the
    // same degree - max. When we add an edge to the solution, the degree of some nearby edges gets
    // decreased but never increased. Therefore matching.size() + M / max is a lower bound on the
    // size of matching, hence we can use it for branching.
    if (matching.size() + M / max >= MIN_SIZE)
    {
        // revert graph to its previous state
        for (const Edge &edge : reduced_edges)
        {
            graph[edge.first][edge.second] = true;
            graph[edge.second][edge.first] = true;
            ++degrees[edge.first];
            ++degrees[edge.second];
        }
        for (const Edge &edge : add_edges)
        {
            matching.pop_back();
            visited[edge.first] = false;
            visited[edge.second] = false;
        }
        M += reduced_edges.size();
        return;
    }

    // Select edge to branch on
    Edge edge;
    while (true)
    {
        edge = connections[idx++];
        // when we add edges, we remove edges that are further in connections list, therefore we have to check
        // if "edge" still exists in the graph, if it doesn't it will not reappear in this branch so we can 
        // safely increment idx and don't go back to it. It is important because thanks to it we don't have 
        // to iterate over all connections both when we choose edge, and when we check bounding condition.
        // all edges at connections[i], where i < idx are removed from the graph
        if (degrees[edge.first] && degrees[edge.second])
        {
            break;
        }
    }
    Node u = edge.first;
    Node v = edge.second;

    // Include edge (u, v)
    {
        EdgeList removed_edges;
        visited[u] = true;
        visited[v] = true;
        NodeList degree0;

        // Remove u and v
        for (short w = 0; w < N; ++w)
        {
            if (graph[u][w])
            {
                graph[w][u] = false;
                graph[u][w] = false;
                removed_edges.push_back({w, u});
                --degrees[w];
                if (!degrees[w])
                {
                    degree0.push_back(w);
                }
            }
            if (graph[v][w])
            {
                graph[w][v] = false;
                graph[v][w] = false;
                removed_edges.push_back({w, v});
                --degrees[w];
                if (!degrees[w])
                {
                    degree0.push_back(w);
                }
            }
        }
        M -= removed_edges.size();

        degrees[u] = 0;
        degrees[v] = 0;

        // Check if the current state leads to maximal matching, if not - don't include the edge
        Bool valid = true;
        // Check all nodes that degree has gone to 0 after adding an edge
        for (const Node node : degree0)
        {
            // If originally this node had more than 1 neighbour
            if (FULL_GRAPH[node].size() > 1 && !visited[node])
            {
                // check if all its neighbours are either in the graph or in the matching
                for (const Node n : FULL_GRAPH[node])
                {
                    if (!visited[n] && !degrees[n])
                    {
                        // If not, don't include the edge
                        valid = false;
                        break;
                    }
                }
            }
            if (!valid)
            {
                break;
            }
        }

        if (valid)
        {
            matching.push_back(edge);
            branch_and_reduce(idx);
            matching.pop_back();
        }

        // add edges that were removed after adding "edge"
        for (const Edge &edge : removed_edges)
        {
            graph[edge.first][edge.second] = true;
            graph[edge.second][edge.first] = true;
            ++degrees[edge.first];
            ++degrees[edge.second];
        }
        M += removed_edges.size();
    }

    // remove "edge", because it was added in the for loop above
    graph[u][v] = false;
    graph[v][u] = false;
    --degrees[u];
    --degrees[v];
    --M;

    visited[u] = false;
    visited[v] = false;

    // Now validity check
    // if degree of u has gone to 0
    if (degrees[u] == 0)
    {
        // check if all its neighbours are either in the graph or in the matching
        for (const Node node : FULL_GRAPH[u])
        {
            if (!visited[node] && !degrees[node])
            {
                // If not, revert graph to its state before invoking this function, and abondon this branch
                graph[u][v] = true;
                graph[v][u] = true;
                ++degrees[u];
                ++degrees[v];
                ++M;
                for (const Edge &edge : reduced_edges)
                {
                    graph[edge.first][edge.second] = true;
                    graph[edge.second][edge.first] = true;
                    ++degrees[edge.first];
                    ++degrees[edge.second];
                }
                for (const Edge &edge : add_edges)
                {
                    matching.pop_back();
                    visited[edge.first] = false;
                    visited[edge.second] = false;
                }
                M += reduced_edges.size();
                return;
            }
        }
    }

    // if degree of v has gone to 0
    if (degrees[v] == 0)
    {
        // check if all its neighbours are either in the graph or in the matching
        for (const Node node : FULL_GRAPH[v])
        {
            if (!visited[node] && !degrees[node])
            {
                // If not, revert graph to its state before invoking this function, and abondon this branch
                graph[u][v] = true;
                graph[v][u] = true;
                ++degrees[u];
                ++degrees[v];
                ++M;
                for (const Edge &edge : reduced_edges)
                {
                    graph[edge.first][edge.second] = true;
                    graph[edge.second][edge.first] = true;
                    ++degrees[edge.first];
                    ++degrees[edge.second];
                }
                for (const Edge &edge : add_edges)
                {
                    matching.pop_back();
                    visited[edge.first] = false;
                    visited[edge.second] = false;
                }
                M += reduced_edges.size();
                return;
            }
        }
    }

    // Exclude edge (u, v)
    // Remove edge (u, v)
    branch_and_reduce(idx);

    // revert the graph to the state before invoking this function
    graph[u][v] = true;
    graph[v][u] = true;
    ++degrees[u];
    ++degrees[v];
    ++M;
    for (const Edge &edge : reduced_edges)
    {
        graph[edge.first][edge.second] = true;
        graph[edge.second][edge.first] = true;
        ++degrees[edge.first];
        ++degrees[edge.second];
    }
    for (const Edge &edge : add_edges)
    {
        matching.pop_back();
        visited[edge.first] = false;
        visited[edge.second] = false;
    }
    M += reduced_edges.size();
}

int main()
{
    std::cin >> N >> M;

    for (short i = 0; i < N; ++i)
    {
        graph.push_back(std::vector<Bool>(N, false));
    }

    degrees.resize(N);
    connections.reserve(M);
    FULL_GRAPH.resize(N);

    for (short i = 0; i < M; ++i)
    {
        short a, b;
        std::cin >> a >> b;
        connections.push_back({a, b});
        graph[a][b] = true;
        graph[b][a] = true;
        FULL_GRAPH[a].push_back(b);
        FULL_GRAPH[b].push_back(a);
        ++degrees[a];
        ++degrees[b];
    }

    // sort connections by decreasing degree because including high degree edges simplyfies the graph early
    // which can faster lead to reduction rules and the graphs is getting smaller more quickly, which
    // speeds up iterations over all remaining nodes / edges
    std::sort(connections.begin(), connections.end(), [&](auto const &a, auto const &b) {
        return degrees[a.first] + degrees[a.second] > degrees[b.first] + degrees[b.second];
    });

    for (const Edge edge : connections)
    {
        DEGREES.push_back(degrees[edge.first] + degrees[edge.second]);
    }

    visited.resize(N);
    branch_and_reduce(0);

    print_result(MIN_MATCHING);
}