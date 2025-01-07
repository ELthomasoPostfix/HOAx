#include "hoax.h"

HOAxParityTwA::HOAxParityTwA(const spot::twa_graph_ptr aut) {
    assert(aut != nullptr);
    assert(this->exp == nullptr);
    assert(this->src == nullptr);
    this->src = aut;
    this->exp = spot::make_twa_graph(aut->get_dict());

    /* In a spot/HOA TwA every state corresponds to an integer state "index".
        Preallocate one new state for every existing state. This ensures there is
        a 1 to 1 mapping from old TwA integer state and new TwA integer state. */
    assert(this->exp->num_states() == 0);
    this->exp->new_states(aut->num_states());

    bdd controllable = spot::get_synthesis_outputs(aut);
    bdd uncontrollable = bdd_variablescomp(controllable);

    /* All of the states in the automaton belong to the odd player. */
    for (unsigned int state = 0; state < aut->num_states(); state++) {
        /* Collect all distinct variables of the out edges as a conjunction. */
        bdd outvars = bddtrue;
        for (auto &edge : aut->out(state))
        outvars &= bdd_variables(edge.cond);
        /* Determine the uncontrollable out variables as a conjunction. */
        bdd unc_uvars = bdd_restrict(outvars, controllable);

        int *indexes = nullptr;
        unsigned int size = 0;
        bdd_var_indexes(unc_uvars, &indexes, &size);

        /* Generate all possible evaluations of the uncontrollable vars.
        Every integer in [0, 2^k] for k uncontrollable variables
        maps to a single evaluation of those variables, based on the
        binary representation of that integer. e.g.
            000  =>  !a & !b & !c
            001  =>  !a & !b & c
            ...
            111  =>  a & b & c
        */
        for (int eval_nr = 0; eval_nr < std::pow(2, size); eval_nr++) {
            bdd eval = bddtrue;
            for (unsigned int shift = 0; indexes && (shift < size); shift++) {
                bdd var = bdd_ithvar(indexes[shift]);
                if ((eval_nr & (1 << shift)) == 0)
                    eval &= !var;
                else
                    eval &= var;
            }

            std::vector<spot::twa_graph::edge_storage_t*> destinations;
            for (auto &edge : aut->out(state)) {
                /* If the edge is still satisfiable after the odd player chooses
                a uncontrollable var evaluation, then it induces a new edge.
                i.e. the even player can still make a move.
                */
                if (bdd_satone(bdd_restrict(edge.cond, eval)) != bddfalse) {
                destinations.push_back(&edge);
                }
            }

            /* The eval of the uncontrollable variables results in none of the
                out transitions having a satisfiable condition. */
            if (destinations.size() == 0)
                continue;

            const unsigned int intermediate = this->exp->new_state();
            unsigned int edge_id = this->exp->new_acc_edge(state, intermediate, eval);
            /* The intermediary transition does not belong to any accepting set. */
            assert(this->exp->edge_storage(edge_id).acc.count() == 0);

            /* For each out edge of the original state, that is still satisfiable
                given the eval of the uncontrollable vars, add an out edge to the
                intermediary state.

                Spot makes the distinction between transitions (labeled with
                a single AP) and edges (labeled with an entire propositional
                formula):
                    https://spot.lre.epita.fr/concepts.html#trans-edge
                So, use `new_edge(...)` instead of `new_transition(...)`?
            */
            for (auto edge : destinations) {
                bdd cond = bdd_restrict(edge->cond, eval);
                edge_id = this->exp->new_acc_edge(intermediate, edge->dst, cond);
                this->exp->edge_storage(edge_id).acc = edge->acc;
            }
        }
        if (indexes) free(indexes);
    }
}

void HOAxParityTwA::set_state_names() {
    auto names = this->exp->get_or_set_named_prop<std::vector<std::string>>("state-names");
    names->resize(this->exp->num_states());
    const unsigned int src_size = this->src->num_states();
    const unsigned int aut_size = this->exp->num_states();
    for (unsigned int i = 0; i < src_size; i++)
        (*names)[i] = ("A" + std::to_string(i));
    for (unsigned int i = src_size; i < aut_size; i++)
        (*names)[i] = ("E" + std::to_string(i));
}

std::set<int> HOAxParityTwA::get_all_states() const {
    std::set<int> states;
    for (unsigned int i = 0; i < this->exp->num_states(); i++)
        states.insert(i);
    return states;
}

std::set<int> HOAxParityTwA::get_even_states() const {
    std::set<int> states;
    for (unsigned int i = this->src->num_states(); i < this->exp->num_states(); i++)
        states.insert(i);
    return states;
}

std::set<int> HOAxParityTwA::get_odd_states() const {
    std::set<int> states;
    for (unsigned int i = 0; i < this->src->num_states(); i++)
        states.insert(i);
    return states;
}

void zielonka(
    std::set<int> *W0,
    std::set<int> *W1,
    const std::set<int> *vertices,
    const std::set<int> *vertices_even,
    const spot::twa_graph_ptr aut,
    const bool parity_max) {

    std::cout << "Zielonka" << std::endl; // TODO: DELETE!!!!
    std::cout << "A = " << *vertices << std::endl; // TODO: DELETE!!!!
    std::cout << "E = " << *vertices_even << std::endl; // TODO: DELETE!!!!

    // TODO: The zielonka algorithm is state-based, but the eHOA benchmarks
    //       are exclusively transition-based!
    //   ==> how to handle this?

    // The sets W0 and W1 are output params, they should be empty initially.
    assert(W0->empty());
    assert(W1->empty());

    /* Base case: no more vertices remain to be checked. */
    if (vertices_even->empty()) {
        /* Any odd vertices not assigned to a winnable set by now are
          "odd player" vertices with no outgoing edges, i.e. sink nodes.
          We assumed no sink nodes existed! */
        assert(vertices->empty());
        return;
    }


    /* (1) Support a player based on the extremum priority's parity. */
    // The min/max priority, depending on the parity condition.
    unsigned int m = parity_max ? 0 : INT_MAX;
    for (int vertex : *vertices_even) {
        m = parity_max ? std::max(m, priority(aut, vertex, parity_max)) :
                         std::min(m, priority(aut, vertex, parity_max));
    }
    // The player to support.
    const int player = m % 2;

    std::cout << "m = " << m << " | player = " << player << std::endl; // TODO: DELETE!!!!

    // The vertices matching the extremum priority.
    std::set<int> M;
    for (int vertex : *vertices_even) {
        if (priority(aut, vertex, parity_max) == m)
            M.insert(vertex);
    }

    std::cout << "M = " << M << std::endl; // TODO: DELETE!!!!

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
        // Recursively solve for (G \ R)
        std::set<int> vertices_rem = *vertices - R;
        std::set<int> vertices_even_rem = *vertices_even - R;
        zielonka(&Wcurr_p1, &Wprev_p1, &vertices_rem, &vertices_even_rem, aut, parity_max);
    }

    if (Wprev_p1.empty()) {
        // W_i = W'_i U R
        *Wcurr_p0 = Wcurr_p1 + R;
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
            // Recursively solve for (G \ S)
            std::set<int> vertices_rem = *vertices - S;
            std::set<int> vertices_even_rem = *vertices_even - S;
            zielonka(&Wcurr_p2, &Wprev_p2, &vertices_rem, &vertices_even_rem, aut, parity_max);
        }

        // W_i = W''_i
        *Wcurr_p0 = std::move(Wcurr_p2);
        // W_(i-1) = W''_(i-1) U S
        *Wprev_p0 = Wprev_p2 + S;
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
    if (attr_rec.size() == (vertices_even->size() + vertices_odd->size())) {
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
    bool edges_empty = true;
    for (auto &edge : aut->out(state)) {
        edges_empty = false;
        if (parity_max)
            p = std::max(p, priority(edge.acc, parity_max));
        else
            p = std::min(p, priority(edge.acc, parity_max));
    }
    /* Sink nodes, vertices with no outgoing edges, are not allowed! */
    assert(!edges_empty);
    return p;
}
