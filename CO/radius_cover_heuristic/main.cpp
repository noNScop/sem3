#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <vector>

using AdjList = std::vector<std::vector<int>>;

constexpr std::chrono::milliseconds TIME_LIMIT{29000};
int MAX_QUEUE = 0;

int calc_edgers(AdjList &adj_list, std::vector<bool> &covered, int k, int node)
{
    if (MAX_QUEUE > 10000)
    {
        return 0;
    }
    int degree = 0;
    std::vector<bool> visited(adj_list.size(), false);
    std::queue<std::pair<int, int>> q;
    q.push({node, 0});
    visited[node] = true;

    while (!q.empty())
    {
        if (q.size() > MAX_QUEUE)
        {
            MAX_QUEUE = q.size();
            if (MAX_QUEUE > 10000)
            {
                return 0;
            }
        }
        auto [curr, dist] = q.front();
        q.pop();
        ++dist;

        for (int neighbor : adj_list[curr])
        {
            if (!visited[neighbor])
            {
                visited[neighbor] = true;
                if (dist == k)
                {
                    bool edger = true;
                    for (int v : adj_list[neighbor])
                    {
                        if (!covered[v] && !visited[v])
                        {
                            edger = false;
                            break;
                        }
                    }
                    if (edger && !covered[neighbor])
                    {
                        ++degree;
                    }
                }
                else
                {
                    q.push({neighbor, dist});
                }
            }
        }
    }
    return degree;
}

int calc_edgers_RS(AdjList &adj_list, std::vector<bool> &covered, int k, int node)
{
    int degree = 0;
    std::vector<bool> visited(adj_list.size(), false);
    std::queue<std::pair<int, int>> q;
    q.push({node, 0});
    visited[node] = true;

    while (!q.empty())
    {
        auto [curr, dist] = q.front();
        q.pop();
        ++dist;

        for (int neighbor : adj_list[curr])
        {
            if (!visited[neighbor])
            {
                visited[neighbor] = true;
                if (adj_list[neighbor].size() == 1 && !covered[neighbor])
                {
                    ++degree;
                }
                if (dist < k)
                {
                    q.push({neighbor, dist});
                }
            }
        }
    }
    return degree;
}

int calc_degree(AdjList &adj_list, std::vector<bool> &covered, int k, int node)
{
    int degree = 0;
    std::vector<bool> visited(adj_list.size(), false);
    std::queue<std::pair<int, int>> q;
    q.push({node, 0});
    visited[node] = true;

    while (!q.empty())
    {
        auto [curr, dist] = q.front();
        q.pop();
        ++dist;

        for (int neighbor : adj_list[curr])
        {
            if (!visited[neighbor])
            {
                visited[neighbor] = true;
                if (!covered[neighbor])
                {
                    ++degree;
                }
                if (dist < k)
                {
                    q.push({neighbor, dist});
                }
            }
        }
    }
    return degree;
}

std::vector<int> get_neighbours(AdjList &adj_list, int k, int node)
{
    std::vector<int> neighbours;
    std::queue<std::pair<int, int>> q;
    std::vector<bool> visited(adj_list.size(), false);
    q.push({node, 0});
    visited[node] = true;

    while (!q.empty())
    {
        auto [curr, dist] = q.front();
        neighbours.push_back(curr);
        q.pop();
        ++dist;

        for (int neighbor : adj_list[curr])
        {
            if (!visited[neighbor])
            {
                visited[neighbor] = true;
                if (dist <= k)
                {
                    q.push({neighbor, dist});
                }
            }
        }
    }
    return neighbours;
}

void bfs(AdjList &adj_list, std::vector<bool> &covered, int k, int node)
{
    std::queue<std::pair<int, int>> q;
    std::vector<bool> visited(adj_list.size(), false);
    q.push({node, 0});
    covered[node] = true;
    visited[node] = true;

    while (!q.empty())
    {
        auto [curr, dist] = q.front();
        q.pop();
        ++dist;

        for (int neighbor : adj_list[curr])
        {
            if (!visited[neighbor])
            {
                visited[neighbor] = true;
                covered[neighbor] = true;
                if (dist < k)
                {
                    q.push({neighbor, dist});
                }
            }
        }
    }
}

int main()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
    int N;
    int M;
    std::cin >> N >> M;

    AdjList adj_list;
    adj_list.resize(N);

    std::vector<bool> covered(N, false);

    std::vector<int> degrees(N, 0);
    std::vector<int> edgers(N, 0);

    for (int i = 0; i < M; ++i)
    {
        int a, b;
        std::cin >> a >> b;
        adj_list[a].push_back(b);
        adj_list[b].push_back(a);
        degrees[a] = adj_list[a].size();
    }
    int k;
    std::cin >> k;

    std::vector<int> ordering(N, 0);
    std::iota(ordering.begin(), ordering.end(), 0);
    std::sort(ordering.begin(), ordering.end(), [&](int a, int b) { return degrees[a] > degrees[b]; });

    for (int i : ordering)
    {
        degrees[i] = calc_degree(adj_list, covered, k, i);
        if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
        {
            break;
        }
        edgers[i] = calc_edgers(adj_list, covered, k, i);
    }

    std::vector<int> ans;
    std::vector<int> challengers(N, 0);
    std::iota(challengers.begin(), challengers.end(), 0);

    while (std::chrono::high_resolution_clock::now() - start_time < TIME_LIMIT)
    {
        int node = 0;
        std::vector<int> new_challengers;
        for (int i : challengers)
        {
            if (degrees[i])
            {
                new_challengers.push_back(i);
                if (edgers[i] == edgers[node])
                {
                    if (degrees[i] > degrees[node])
                    {
                        node = i;
                    }
                }
                else
                {
                    if (edgers[i] > edgers[node])
                    {
                        node = i;
                    }
                }
            }
        }
        if (degrees[node] == 0)
        {
            break;
        }
        challengers = new_challengers;
        ans.push_back(node);
        bfs(adj_list, covered, k, node);

        std::vector<int> neighbours;
        if (N >= 100000 && k > 1)
        {
            neighbours = get_neighbours(adj_list, 2 * k - 1, node);
        }
        else
        {
            neighbours = get_neighbours(adj_list, 2 * k, node);
        }
        for (int i : neighbours)
        {
            degrees[i] = calc_degree(adj_list, covered, k, i);
            if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
            {
                break;
            }
            edgers[i] = calc_edgers(adj_list, covered, k, i);
        }
    }
    std::sort(ordering.begin(), ordering.end(), [&](int a, int b) {
        if (edgers[a] == edgers[b])
        {
            return degrees[a] > degrees[b];
        }
        return edgers[a] > edgers[b];
    });

    for (int node : ordering)
    {
        if (!covered[node])
        {
            ans.push_back(node);
            bfs(adj_list, covered, k, node);
        }
    }

    if (N >= 100000 && k > 1)
    {
        covered = std::vector<bool>(N, false);
        for (int i = 0; i < N; ++i)
        {
            degrees[i] = calc_degree(adj_list, covered, k, i);
            if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
            {
                break;
            }
            edgers[i] = calc_edgers(adj_list, covered, k, i);
            if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
            {
                break;
            }
        }

        std::vector<int> new_ans;
        std::vector<int> challengers(N, 0);
        std::iota(challengers.begin(), challengers.end(), 0);

        while (std::chrono::high_resolution_clock::now() - start_time < TIME_LIMIT)
        {
            int node = 0;
            std::vector<int> new_challengers;
            for (int i : challengers)
            {
                if (degrees[i])
                {
                    new_challengers.push_back(i);
                    if (edgers[i] == edgers[node])
                    {
                        if (degrees[i] > degrees[node])
                        {
                            node = i;
                        }
                    }
                    else
                    {
                        if (edgers[i] > edgers[node])
                        {
                            node = i;
                        }
                    }
                }
            }
            if (degrees[node] == 0)
            {
                break;
            }
            challengers = new_challengers;
            new_ans.push_back(node);
            if (new_ans.size() > ans.size())
            {
                break;
            }
            bfs(adj_list, covered, k, node);

            std::vector<int> neighbours;
            neighbours = get_neighbours(adj_list, 2 * k, node);
            for (int i : neighbours)
            {
                degrees[i] = calc_degree(adj_list, covered, k, i);
                if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
                {
                    break;
                }
                edgers[i] = calc_edgers(adj_list, covered, k, i);
                if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
                {
                    break;
                }
            }
        }
        std::vector<int> ordering(N, 0);
        std::iota(ordering.begin(), ordering.end(), 0);
        std::sort(ordering.begin(), ordering.end(), [&](int a, int b) {
            if (edgers[a] == edgers[b])
            {
                return degrees[a] > degrees[b];
            }
            return edgers[a] > edgers[b];
        });

        for (int node : ordering)
        {
            if (!covered[node])
            {
                new_ans.push_back(node);
                bfs(adj_list, covered, k, node);
            }
        }
        if (new_ans.size() < ans.size())
        {
            ans = new_ans;
        }
    }

    std::vector<int> best_ans = ans;
    std::random_device rd; // Seed
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, N);
    int counter = 0;

    while (std::chrono::high_resolution_clock::now() - start_time < TIME_LIMIT)
    {
        counter = (counter + 1) % N;
        int node = ans[dis(gen) % ans.size()];
        std::vector<int> neighbours = get_neighbours(adj_list, counter, node);
        if (neighbours.size() > 0.1 * N || counter == 4 * k)
        {
            counter = 0;
        }
        std::vector<int> to_remove;
        for (int i : ans)
        {
            if (std::find(neighbours.begin(), neighbours.end(), i) != neighbours.end())
            {
                to_remove.push_back(i);
            }
        }

        for (int x : to_remove)
        {
            ans.erase(std::remove(ans.begin(), ans.end(), x), ans.end());
        }
        std::vector<bool> covered(N, false);

        for (int n : ans)
        {
            bfs(adj_list, covered, k, n);
        }

        std::vector<int> challengers;
        for (int i = 0; i < N; ++i)
        {
            if (!covered[i])
            {
                challengers.push_back(i);
            }
        }

        std::vector<int> degrees(N, 0);
        std::vector<int> edgers(N, 0);

        for (int i : challengers)
        {
            degrees[i] = calc_degree(adj_list, covered, k, i);
            if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
            {
                break;
            }
            edgers[i] = calc_edgers_RS(adj_list, covered, k, i);
            if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
            {
                break;
            }
        }

        while (std::chrono::high_resolution_clock::now() - start_time < TIME_LIMIT)
        {
            int node = 0;
            std::vector<int> new_challengers;
            for (int i : challengers)
            {
                if (degrees[i])
                {
                    new_challengers.push_back(i);
                    if (edgers[i] == edgers[node])
                    {
                        if (degrees[i] > degrees[node])
                        {
                            node = i;
                        }
                    }
                    else
                    {
                        if (edgers[i] > edgers[node])
                        {
                            node = i;
                        }
                    }
                }
            }
            if (degrees[node] == 0)
            {
                break;
            }
            challengers = new_challengers;
            ans.push_back(node);
            bfs(adj_list, covered, k, node);

            std::vector<int> neighbours = get_neighbours(adj_list, 2 * k, node);
            for (int i : neighbours)
            {
                degrees[i] = calc_degree(adj_list, covered, k, i);
                edgers[i] = calc_edgers_RS(adj_list, covered, k, i);
            }
        }
        for (int node = 0; node < N; ++node)
        {
            if (!covered[node])
            {
                ans.push_back(node);
                bfs(adj_list, covered, k, node);
            }
        }
        // std::cout << ans.size() << std::endl;

        if (ans.size() < best_ans.size())
        {
            best_ans = ans;
        }
        else
        {
            ans = best_ans;
        }

        if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
        {
            break;
        }

        node = ans[dis(gen) % ans.size()];
        neighbours = get_neighbours(adj_list, counter, node);
        if (neighbours.size() > 0.1 * N || counter == 4 * k)
        {
            counter = 0;
        }
        to_remove.clear();
        for (int i : ans)
        {
            if (std::find(neighbours.begin(), neighbours.end(), i) != neighbours.end())
            {
                to_remove.push_back(i);
            }
        }

        for (int x : to_remove)
        {
            ans.erase(std::remove(ans.begin(), ans.end(), x), ans.end());
        }
        covered = std::vector<bool>(N, false);

        for (int n : ans)
        {
            bfs(adj_list, covered, k, n);
        }

        challengers.clear();
        for (int i = 0; i < N; ++i)
        {
            if (!covered[i])
            {
                challengers.push_back(i);
            }
        }

        degrees = std::vector<int>(N, 0);
        edgers = std::vector<int>(N, 0);

        for (int i : challengers)
        {
            degrees[i] = calc_degree(adj_list, covered, k, i);
            if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
            {
                break;
            }
            edgers[i] = calc_edgers(adj_list, covered, k, i);
            if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
            {
                break;
            }
        }

        while (std::chrono::high_resolution_clock::now() - start_time < TIME_LIMIT)
        {
            int node = 0;
            std::vector<int> new_challengers;
            for (int i : challengers)
            {
                if (degrees[i])
                {
                    new_challengers.push_back(i);
                    if (edgers[i] == edgers[node])
                    {
                        if (degrees[i] > degrees[node])
                        {
                            node = i;
                        }
                    }
                    else
                    {
                        if (edgers[i] > edgers[node])
                        {
                            node = i;
                        }
                    }
                }
            }
            if (degrees[node] == 0)
            {
                break;
            }
            challengers = new_challengers;
            ans.push_back(node);
            bfs(adj_list, covered, k, node);

            std::vector<int> neighbours = get_neighbours(adj_list, 2 * k, node);
            for (int i : neighbours)
            {
                degrees[i] = calc_degree(adj_list, covered, k, i);
                if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
                {
                    break;
                }
                edgers[i] = calc_edgers(adj_list, covered, k, i);
                if (std::chrono::high_resolution_clock::now() - start_time > TIME_LIMIT)
                {
                    break;
                }
            }
        }
        for (int node = 0; node < N; ++node)
        {
            if (!covered[node])
            {
                ans.push_back(node);
                bfs(adj_list, covered, k, node);
            }
        }
        // std::cout << ans.size() << std::endl;

        if (ans.size() < best_ans.size())
        {
            best_ans = ans;
        }
        else
        {
            ans = best_ans;
        }
    }

    std::cout << ans.size() << std::endl;
    for (int node : ans)
    {
        std::cout << node << " ";
    }
    std::cout << std::endl;
}