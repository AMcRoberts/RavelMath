// Probe (not a proof/test): computes the EXACT integer adjacency
// submatrix of each of the three Class-II adjacent-swap neighbors'
// dominant recurrent core (class_ii_neighbor_dominant_core_states),
// and its exact characteristic polynomial via charpoly_faddeev_leverrier,
// to check whether it matches the quadratic/linear relation
// lean/class_ii_neighbor_dominance.lean's neighbor{0,1,2}_shell_below_*
// theorems expect (lambda+1/lambda=t+2, lambda^2=t*(t+2), lambda=k
// respectively). Since the dominant core catalogue is a-INDEPENDENT
// (same fixed 15/17/39-state set for every 3<=a<=8, per the file's own
// comment), this only needs to run once per neighbor, at any a in that
// verified range -- not per-a.

#include <cstdio>
#include <set>
#include <vector>

#include "math/charpoly.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/pisot_substitution_properties.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"

using namespace ravel;
using Matrix = std::vector<std::vector<long long>>;

namespace {

FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image) {
        for (const auto letter : substitution[image]) {
            ++matrix[static_cast<std::size_t>(letter)][image];
        }
    }
    return matrix;
}

SNode<3> to_snode(
        const std::tuple<long long, std::vector<long long>, long long>& tuple) {
    SNode<3> node;
    node.i = std::get<0>(tuple);
    node.j = std::get<2>(tuple);
    const auto& x = std::get<1>(tuple);
    for (std::size_t c = 0; c < 3; ++c) node.x[c] = x[c];
    return node;
}

}  // namespace

int main() {
    const long long a = 5;  // any value in the certified 3<=a<=8 range
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);

    for (std::size_t neighbor = 0; neighbor < 3; ++neighbor) {
        const auto& tau = neighbors[neighbor].substitution;
        SubstitutionRule rule(tau);
        const auto subst = make_substitution<3>(rule, spectral.beta);
        const auto candidates = search_D_cont<3>(subst, 2);
        std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
        for (const auto& c : candidates)
            d_cont.emplace_back(c.i, std::vector<long long>(c.x.begin(), c.x.end()), c.j);

        ContactBoundaryLimits limits;
        limits.max_corona_rounds = static_cast<int>(a) + 4;
        const auto value =
            compute_contact_boundary_dispatch(rule, spectral.beta, 0.0, d_cont, limits);

        const auto graph = WeightedDigraph::from_dense(value.gb_matrix);
        const auto sccs = tarjan_scc(graph);

        const auto core_states = class_ii_neighbor_dominant_core_states(neighbor);

        // Reconstruct the node list in the SAME order compute_contact_
        // boundary_dispatch used to build value.gb_matrix (value.nodes),
        // find which Tarjan component's node-INDEX-SET matches the
        // catalogued dominant core by translating indices to SNode<3> via
        // value.nodes and comparing as a set -- independent re-derivation,
        // not trusting index order blindly.
        bool found = false;
        for (const auto& component : sccs) {
            std::set<SNode<3>> component_states;
            for (auto idx : component) component_states.insert(to_snode(value.boundary_nodes.at(idx)));
            if (component_states != core_states) continue;
            found = true;
            std::printf("neighbor %zu: dominant core matched, %zu states\n",
                        neighbor, component.size());
            auto sorted = component;
            std::sort(sorted.begin(), sorted.end());
            const std::size_t n = sorted.size();
            Matrix sub(n, std::vector<long long>(n, 0));
            for (std::size_t r = 0; r < n; ++r)
                for (std::size_t c = 0; c < n; ++c)
                    sub[r][c] = value.gb_matrix[sorted[r]][sorted[c]];
            const auto poly = mathlib::charpoly_faddeev_leverrier(sub);
            std::printf("  charpoly coeffs (low..high): ");
            for (std::size_t i = 0; i < poly.coeffs_.size(); ++i)
                std::printf("%s ", mathlib::str(poly.coeffs_[i]).c_str());
            std::printf("\n");
            break;
        }
        if (!found) std::printf("neighbor %zu: dominant core NOT found among SCCs!\n", neighbor);
    }
}
