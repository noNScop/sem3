#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <sys/types.h>
#include <vector>

template<class C>
void ranges_reverse(C& container) {
    std::reverse(std::begin(container), std::end(container));
}

template<class C, class It>
void ranges_copy(C const& input, It out_iter) {
    std::copy(std::begin(input), std::end(input), out_iter);
}

template<class C, class It>
void ranges_move(C const& input, It out_iter) {
    std::move(std::begin(input), std::end(input), out_iter);
}

int const INF = std::numeric_limits<int>::max();

struct Vec2 {
    int x = 0;
    int y = 0;
};

class Graph;

class Node {
public:
    explicit Node(int i)
        : id(i) { }

    // convert to representation from input
    int index() const {
        return id;
    }

    // convert to row, column representation
    Vec2 decode(Graph const& g) const;

    // Another no C++20 stuff, in C++20 it would just be:
    //   bool operator==(Node const& n) const = default;
    //   std::strong_ordering operator<=>(Node const& n) const = default;
    bool operator==(Node const& n) const {
        return id == n.id;
    }
    bool operator!=(Node const& n) const {
        return id != n.id;
    }
    bool operator<(Node const& n) const {
        return id < n.id;
    }
    bool operator<=(Node const& n) const {
        return id <= n.id;
    }
    bool operator>(Node const& n) const {
        return id > n.id;
    }
    bool operator>=(Node const& n) const {
        return id >= n.id;
    }

private:
    int id;
};

struct Edge {
    Node destination;
    int weight;
};

using Path = std::vector<Node>;

namespace std {
template<>
struct hash<Node> {
    size_t operator()(Node const& node) const {
        return std::hash<int> {}(node.index());
    }
};
}

struct Individual {
    Path path;
    int cost;
};

class Graph {
public:
    Graph(Vec2 size)
        : m_size(size)
        , gen(rd())
        , uni_real_dist(0.0, 1.0) {
        m_adjacency_list.resize(size.x * size.y);
    }

    Vec2 size() const {
        return m_size;
    }

    void add_edge(Node start, Node end, int weight) {
        assert(start.index() < m_adjacency_list.size());
        m_adjacency_list[start.index()].push_back({ end, weight });
    }

    std::vector<Edge> const& get_edges(Node node) const {
        assert(node.index() < m_adjacency_list.size());
        return m_adjacency_list[node.index()];
    }

    // returns path nodes, including start and goal
    std::vector<Node> dijkstra(std::vector<bool>& visited, Node start, Node goal) const;

    std::vector<Node> random_dijkstra(Node start, Node goal, int max_perturbation, float pert_prob);

    int calculate_path_cost(std::vector<Node> const& nodes) const;

    size_t node_count() const {
        return m_adjacency_list.size();
    }

    bool is_valid_a_to_b(Node idx) const {
        auto [x, y] = idx.decode(*this);
        return (y < m_size.y - 1 && x < m_size.x - 1) || idx.index() == m_size.x - 1;
    }

    bool is_valid_b_to_c(Node idx) const {
        auto [x, y] = idx.decode(*this);
        return y > 0;
    }

    auto node_a() const {
        return Node(0);
    }
    auto node_b() const {
        return Node(m_size.x - 1);
    }
    auto node_c() const {
        return Node((m_size.y - 1) * m_size.x);
    }

private:
    Vec2 m_size;
    std::vector<std::vector<Edge>> m_adjacency_list;

    // random generators setup
    std::random_device rd;
    std::default_random_engine gen;
    std::uniform_real_distribution<float> uni_real_dist;
};

Vec2 Node::decode(Graph const& g) const {
    return { id % g.size().x, id / g.size().x };
}

std::vector<Node> Graph::dijkstra(std::vector<bool>& visited, Node start, Node goal) const {
    std::vector<int> costs(node_count(), INF);
    std::vector<Node> prev(node_count(), Node(INF));
    costs[start.index()] = 0;

    using Pair = std::pair<int, Node>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<>> pq;
    pq.push({ 0, start });

    while (!pq.empty()) {
        int cost = pq.top().first;
        Node node = pq.top().second;
        pq.pop();

        if (cost > costs[node.index()]) {
            continue;
        }

        if (node == goal) {
            std::vector<Node> tmp;
            while (node.index() != INF) {
                visited[node.index()] = true;
                tmp.push_back(node);
                node = prev[node.index()];
            }
            assert(!tmp.empty());

            ranges_reverse(tmp);
            return tmp;
        }

        for (Edge const& edge : get_edges(node)) {
            if (!visited[edge.destination.index()]) {
                int new_cost = cost + edge.weight;
                if (new_cost < costs[edge.destination.index()]) {
                    costs[edge.destination.index()] = new_cost;
                    pq.push({ new_cost, edge.destination });
                    prev[edge.destination.index()] = node;
                }
            }
        }
    }
    // std::cerr << "no path found" << std::endl;
    return {};
}

std::vector<Node> Graph::random_dijkstra(Node start, Node goal, int perturbation, float pert_prob) {
    std::vector<int> costs(node_count(), INF);
    std::vector<Node> prev(node_count(), Node(INF));
    costs[start.index()] = 0;

    using Pair = std::pair<int, Node>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<>> pq;
    pq.push({ 0, start });

    while (!pq.empty()) {
        int cost = pq.top().first;
        Node node = pq.top().second;
        pq.pop();

        if (cost > costs[node.index()]) {
            continue;
        }

        if (node == goal) {
            std::vector<Node> tmp;
            while (node.index() != INF) {
                tmp.push_back(node);
                node = prev[node.index()];
            }
            assert(!tmp.empty());

            ranges_reverse(tmp);
            return tmp;
        }

        for (Edge const& edge : get_edges(node)) {
            int new_cost = cost + edge.weight;

            if (uni_real_dist(gen) < pert_prob) {
                new_cost += perturbation;
            }

            if (new_cost < costs[edge.destination.index()]) {
                costs[edge.destination.index()] = new_cost;
                pq.push({ new_cost, edge.destination });
                prev[edge.destination.index()] = node;
            }
        }
    }
    // std::cerr << "no path found" << std::endl;
    return {};
}
int Graph::calculate_path_cost(std::vector<Node> const& nodes) const {
    std::vector<bool> visited(node_count(), false);
    if (nodes.size() == 0) {
        return INF;
    }
    int cost = 0;
    for (int i = 0; i < nodes.size() - 1; ++i) {
        visited[nodes[i].index()] = true;
        for (Edge const& edge : get_edges(nodes[i])) {
            if (edge.destination == nodes[i + 1]) {
                cost += edge.weight;
                break;
            }
        }
    }
    auto path_bc = dijkstra(visited, node_b(), node_c());

    if (path_bc.size() == 0) {
        return INF;
    }
    else {
        for (int i = 0; i < path_bc.size() - 1; ++i) {
            for (Edge const& edge : get_edges(path_bc[i])) {
                if (edge.destination == path_bc[i + 1]) {
                    cost += edge.weight;
                    break;
                }
            }
        }
        return cost;
    }
}

constexpr std::chrono::milliseconds TIME_LIMIT { 29900 };

Individual run_evolution(Graph& graph, Node b, std::chrono::high_resolution_clock::time_point start_time, int avg_weight) {
    int min = avg_weight * 0.05;
    int max = avg_weight * 0.2;
    float probability = 0.01;
    int perturbation = min;
    auto path_ab = graph.random_dijkstra(graph.node_a(), graph.node_b(), perturbation, probability);
    auto cost = graph.calculate_path_cost(path_ab);
    Individual the_best{std::move(path_ab), cost};

    while (std::chrono::high_resolution_clock::now() - start_time < TIME_LIMIT) {
        if (perturbation >= max) {
            perturbation = min;
            probability += 0.01;
            if (probability > 0.2) {
                probability = 0.01;
            }
        }
        auto path_ab = graph.random_dijkstra(graph.node_a(), graph.node_b(), ++perturbation, probability);
        auto cost = graph.calculate_path_cost(path_ab);
        if (cost < the_best.cost) {
            the_best.path = std::move(path_ab);
            the_best.cost = cost;
        }
    }
    std::vector<bool> visited(graph.node_count(), false);
    the_best.path.pop_back();
    for (int i = 0; i < the_best.path.size(); ++i) {
        visited[the_best.path[i].index()] = true;
    }
    auto path_bc = graph.dijkstra(visited, graph.node_b(), graph.node_c());
    ranges_move(path_bc, std::back_inserter(the_best.path));

    // auto end_time = std::chrono::high_resolution_clock::now();
    // auto duration_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // std::cerr << duration_milliseconds.count() << " milliseconds" << std::endl;

    return the_best;
}

int main(int argc, char const* argv[]) {
    auto start_time = std::chrono::high_resolution_clock::now();
    bool score_mode = argc > 1 && std::string(argv[1]) == "score";

    int N, M, A;
    std::cin >> N >> M >> A;

    auto a = Node(0);
    auto b = Node(M - 1);
    auto c = Node((N - 1) * M);

    Graph graph({ M, N });
    int avg_weight = 0;
    int u, v, w;
    for (int i = 0; i < A; ++i) {
        std::cin >> u >> v >> w;
        graph.add_edge(Node(u), Node(v), w);
        avg_weight += w;
    }
    avg_weight /= graph.node_count();

    auto [ans, cost] = run_evolution(graph, b, start_time, avg_weight);
    std::cerr << "Found path with cost: " << cost << std::endl;

    if (score_mode) {
        std::cout << graph.calculate_path_cost(ans) << std::endl;
    }
    else {
        std::cout << ans.size() << std::endl;
        for (auto num : ans) {
            std::cout << num.index() << " ";
        }
        std::cout << std::endl;
    }

    assert(ans.front() == a);
    assert(ans.back() == c);

    return 0;
}