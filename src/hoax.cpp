#include "hoax.h"

void zielonka(
    std::set<int> *W0,
    std::set<int> *W1,
    const std::vector<int> *vertices,
    const std::vector<int> *vertices_even,
    const std::vector<int> *edges,
    std::function<int(int)> priority_fun) {

    // TODO: The zielonka algorithm is state-based, but the eHOA benchmarks
    //       are exclusively transition-based!
    //   ==> how to handle this?

    // The sets W0 and W1 are output params, they should be empty initially.
    assert(W0->empty());
    assert(W1->empty());

    // Base case: no more vertices remain to be checked.
    if (vertices->empty())
        return;


    int m = INT_MAX;
    for (int vertex : *vertices) {
        int priority = priority_fun(vertex);
        if (priority < m)
            m = priority;
    }

    std::set<int> M;
    for (int vertex : *vertices) {
        int priority = priority_fun(vertex);
        if (priority == m)
            M.insert(vertex);
    }

    std::vector<int> vertices_odd;
    for (int vertex : *vertices) {
        // TODO: Implement the membership check!
        // if (vertex not in vertices_even)
        //     vertices_odd.push_back(vertex);
    }

    const int depth_max = vertices->size();
    const int player = m % 2;
    std::set<int> R = {};
    attractor(&R, &vertices_odd, vertices_even, edges, &M, depth_max, player);

    auto Wcurr_p0 = player == PEVEN ? W0 : W1;  // W_i     = i == 0 ? W0 : W1
    auto Wprev_p0 = player == PEVEN ? W1 : W0;  // W_(i-1) = i == 0 ? W1 : W0

    // The order of the result sets depends on the current player, i.
    std::set<int> Wcurr_p1 = {};  // W'_i
    std::set<int> Wprev_p1 = {};  // W'_(i-1)

    {
        std::vector<int> vertices_rem = {}; // TODO: Compute (G \ R) properly!!!!!!!!!!!!!!!!!!!!!!!!!
        std::vector<int> vertices_even_rem = {};    // TODO: Compute (G \ R) properly!!!!!!!!!!!!!!!!!!!!!!!!!
        std::vector<int> edges_rem = {};    // TODO: Compute (G \ R) properly!!!!!!!!!!!!!!!!!!!!!!!!!
        zielonka(&Wcurr_p1, &Wprev_p1, &vertices_rem, &vertices_even_rem,
                 &edges_rem, priority_fun);
    }

    if (Wprev_p1.empty()) {
        // W_i = W'_i U R
        std::set_union(
            Wcurr_p1.begin(), Wcurr_p1.end(),
            R.begin(), R.end(),
            std::inserter(*Wcurr_p0, Wcurr_p0->end()));
        // W_(i-1) = emptyset
        assert(Wprev_p0->empty());
    } else {
        const int player_other = (player + 1) % 2;
        std::set<int> S = {};
        attractor(&S, &vertices_odd, vertices_even, edges, &Wprev_p1, depth_max, player_other);

        // The order of the result sets depends on the current player, i.
        std::set<int> Wcurr_p2 = {};  // W''_i
        std::set<int> Wprev_p2 = {};  // W''_(i-1)

        {
            std::vector<int> vertices_rem = {}; // TODO: Compute (G \ S) properly!!!!!!!!!!!!!!!!!!!!!!!!!
            std::vector<int> vertices_even_rem = {};    // TODO: Compute (G \ S) properly!!!!!!!!!!!!!!!!!!!!!!!!!
            std::vector<int> edges_rem = {};    // TODO: Compute (G \ S) properly!!!!!!!!!!!!!!!!!!!!!!!!!
            zielonka(&Wcurr_p2, &Wprev_p2, &vertices_rem, &vertices_even_rem,
                     &edges_rem, priority_fun);
        }

        // W_i = W''_i
        *Wcurr_p0 = std::move(Wcurr_p2);
        // W_(i-1) = W''_(i-1) U S
        std::set_union(
            Wprev_p2.begin(), Wprev_p2.end(),
            S.begin(), S.end(),
            std::inserter(*Wprev_p0, Wprev_p0->end()));
    }
}

void attractor(
    std::set<int> *attr,
    const std::vector<int> *vertices_odd,
    const std::vector<int> *vertices_even,
    const std::vector<int> *edges,
    const std::set<int> *T,
    const int k,
    const int i) {

    if (k == 0) {
        attr->insert(T->begin(), T->end());
        return;
    }

    std::set<int> attr_rec;
    const std::vector<int> *vertices;

    attractor(&attr_rec, vertices_odd, vertices_even, edges, T, k-1, i);

    vertices = i == PEVEN ? vertices_even : vertices_odd;
    for (int vertex : *vertices) {
        // TODO: Get access to the actual edges!!!!
        for (int edge : *edges) {
            int src = vertex;  // TODO: How to access transition.src ???
            int dst = vertex;  // TODO: How to access transition.dst ???
            if (vertex == src && contains(&attr_rec, dst)) {
                attr->insert(vertex);
                break;
            }
        }
    }

    vertices = i == PEVEN ? vertices_odd : vertices_even;
    for (int vertex : *vertices) {
        bool isin_all_attractors = true;
        // TODO: Get access to the actual edges!!!!
        for (int edge : *edges) {
            int src = vertex;  // TODO: How to access transition.src ???
            int dst = vertex;  // TODO: How to access transition.dst ???
            if (vertex != src)
                continue;
            if (!contains(&attr_rec, dst)) {
                isin_all_attractors = false;
                break;
            }
        }
        if (isin_all_attractors)
            attr->insert(vertex);
    }

    attr->insert(attr_rec.begin(), attr_rec.end());
}
