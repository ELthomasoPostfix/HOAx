#include "hoax.h"

void zielonka(
    std::set<int> *W0,
    std::set<int> *W1,
    const std::set<int> *vertices,
    const std::set<int> *vertices_even,
    const spot::twa_graph_ptr aut,
    const bool parity_max) {

    // TODO: The zielonka algorithm is state-based, but the eHOA benchmarks
    //       are exclusively transition-based!
    //   ==> how to handle this?

    // The sets W0 and W1 are output params, they should be empty initially.
    assert(W0->empty());
    assert(W1->empty());

    // Base case: no more vertices remain to be checked.
    if (vertices->empty())
        return;


    /* (1) Support a player based on the extremum priority's parity. */
    // The min/max priority, depending on the parity condition.
    unsigned int m = parity_max ? INT_MIN : INT_MAX;
    for (int vertex : *vertices) {
        m = parity_max ? std::max(m, priority(aut, vertex, parity_max)) :
                         std::min(m, priority(aut, vertex, parity_max));
    }
    // The player to support.
    const int player = m % 2;

    // The vertices matching the extremum priority.
    std::set<int> M;
    for (int vertex : *vertices) {
        if (priority(aut, vertex, parity_max) == m)
            M.insert(vertex);
    }

    // All remaining odd/Adam vertices.
    std::set<int> vertices_odd;
    for (int vertex : *vertices) {
        if (!contains(vertices_even, vertex))
            vertices_odd.insert(vertex);
    }

    const int depth_max = vertices->size();
    std::set<int> R = {};
    attractor(&R, &vertices_odd, vertices_even, aut, &M, depth_max, player);

    auto Wcurr_p0 = player == PEVEN ? W0 : W1;  // W_i     = i == 0 ? W0 : W1
    auto Wprev_p0 = player == PEVEN ? W1 : W0;  // W_(i-1) = i == 0 ? W1 : W0

    // The order of the result sets depends on the current player, i.
    std::set<int> Wcurr_p1 = {};  // W'_i
    std::set<int> Wprev_p1 = {};  // W'_(i-1)

    {
        std::set<int> vertices_rem = {}; // TODO: Compute (G \ R) properly!!!!!!!!!!!!!!!!!!!!!!!!!
        std::set<int> vertices_even_rem = {};    // TODO: Compute (G \ R) properly!!!!!!!!!!!!!!!!!!!!!!!!!
        zielonka(&Wcurr_p1, &Wprev_p1, &vertices_rem, &vertices_even_rem, aut, parity_max);
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
        attractor(&S, &vertices_odd, vertices_even, aut, &Wprev_p1, depth_max, player_other);

        // The order of the result sets depends on the current player, i.
        std::set<int> Wcurr_p2 = {};  // W''_i
        std::set<int> Wprev_p2 = {};  // W''_(i-1)

        {
            std::set<int> vertices_rem = {}; // TODO: Compute (G \ S) properly!!!!!!!!!!!!!!!!!!!!!!!!!
            std::set<int> vertices_even_rem = {};    // TODO: Compute (G \ S) properly!!!!!!!!!!!!!!!!!!!!!!!!!
            zielonka(&Wcurr_p2, &Wprev_p2, &vertices_rem, &vertices_even_rem, aut, parity_max);
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
    const std::set<int> *vertices_odd,
    const std::set<int> *vertices_even,
    const spot::twa_graph_ptr aut,
    const std::set<int> *T,
    const int k,
    const int i) {
    assert(k >= 0); // Avoid invalid attractor recursion depth.
    assert(i == PEVEN || i == PODD); // Avoid invalid player.

    // Attr_i^0(G, T) = T
    if (k == 0) {
        attr->insert(T->begin(), T->end());
        return;
    }

    std::set<int> attr_rec;
    const std::set<int> *vertices;

    // Attr_i^(k-1)(G, T)
    attractor(&attr_rec, vertices_odd, vertices_even, aut, T, k-1, i);

    // Fixpoint reached, just pass up the fixpoint set/recursive result.
    // TODO: A fixpoint could be reached before k=0, check for a fixpoint
    //       and possibly quit early?
    //   ==> Implement the attractors with a while loop instead of recursion!
    // TODO: Check that fixpoint check actually works.
    if (attr_rec.size() == aut->num_states()) {
        *attr = std::move(*T);
        return;
    }

    // Add all vertices where the player i can choose to enter the
    // attractor set themselves.
    vertices = i == PEVEN ? vertices_even : vertices_odd;
    for (int vertex : *vertices) {
        for (auto edge : aut->out(vertex)) {
            // The edge is an out edge of vertex, so vertex should be the src
            assert(vertex == edge.src);

            if (contains(&attr_rec, edge.dst)) {
                attr->insert(vertex);
                break;
            }
        }
    }

    // Add all vertices where the player i can force the other player to enter
    // the attractor set.
    vertices = i == PEVEN ? vertices_odd : vertices_even;
    for (int vertex : *vertices) {
        bool can_avoid_attractors = true;
        for (auto edge : aut->out(vertex)) {
            // The edge is an out edge of vertex, so vertex should be the src
            assert(vertex == edge.src);

            if (!contains(&attr_rec, edge.dst)) {
                can_avoid_attractors = false;
                break;
            }
        }
        if (can_avoid_attractors)
            attr->insert(vertex);
    }

    attr->insert(attr_rec.begin(), attr_rec.end());
}

unsigned int priority(const spot::acc_cond::mark_t &mark, const bool parity_max) {
    unsigned int p;
    if (parity_max)
        p = mark.max_set();
    else
        p = mark.min_set();

    /* Spot's `mark_t.max_set()` and `mark_t.min_set()` return 0 when
      an edge is not part of any transition set.
      ==> A transition MUST be part of some acceptance set!
    */
    assert(p > 0);
    return p - 1;
}

unsigned int priority(const spot::twa_graph_ptr aut, const unsigned int state, const bool parity_max) {
    unsigned int p = parity_max ? 0 : UINT_MAX;
    for (auto &edge : aut->out(state)) {
        if (parity_max)
            p = std::max(p, priority(edge.acc, parity_max));
        else
            p = std::min(p, priority(edge.acc, parity_max));
    }
    return p;
}
