// The complete general theory, exercised end to end with NO case
// splitting in the calling code: every entry below -- terminating,
// cyclotomic-padded, non-unit, and eventually-periodic alike -- goes
// through the exact same call. This is the "doesn't break" property
// demonstrated directly, not asserted.

#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/proof/canonical_parent_role_catalogue.hpp"
#include "ravel/proof/general_generator_theorem.hpp"

using namespace mathlib;

QBetaRing ring_from_low_first(const std::vector<long long>& low_first) {
    const std::size_t d = low_first.size();
    PolyZ p; p.ensure_size(d + 1);
    for (std::size_t i = 0; i < d; ++i) set_si(p.coeff(d - 1 - i), low_first[i]);
    set_si(p.coeff(d), 1);
    return QBetaRing(p);
}

void run(const std::string& name, std::vector<long long> low_first, std::size_t expected_primitive) {
    auto R = ring_from_low_first(low_first);
    auto beta_I = isolate_beta(R);
    auto c = ravel::proof::derive_general_generator_theorem(R, beta_I);
    auto forest = ravel::proof::derive_canonical_parent_role_catalogue(R, beta_I);
    auto holonomy = ravel::proof::derive_parent_role_holonomy(forest);
    auto regularity = ravel::proof::derive_parent_role_regularity(forest);
    auto composition = ravel::proof::derive_parent_role_composition(forest);
    auto closure = ravel::proof::derive_parent_role_word_closure(forest, 10, 2);
    std::cout << name << ": digits.size=" << c.digits.size()
              << " raw=" << c.raw_defect_classes << " primitive=" << c.primitive_generator_count
              << " proved=" << c.proved << "\n";
    assert(c.proved);
    assert(c.primitive_generator_count == expected_primitive);
    assert(forest.proved);
    assert(holonomy.proved);
    assert(holonomy.edge_count == forest.edges.size());
    assert(!holonomy.components.empty());
    assert(regularity.proved);
    assert(composition.proved);
    assert(composition.composable_pairs == composition.closed_pairs + composition.missing_pairs);
    assert(closure.proved);
    assert(closure.zero_net_witnesses_verified);
    assert(closure.zero_net_witnesses.size() == closure.zero_net_pairs);
    assert(forest.role_count == forest.alphabet_size * forest.alphabet_size);
    std::size_t parent_occurrences = 0;
    for (const auto& ps : forest.parents) parent_occurrences += ps.size();
    std::size_t expected_edges = 0;
    for (const auto& ps : forest.parents)
        for (const auto& qs : forest.parents) expected_edges += ps.size() * qs.size();
    assert(parent_occurrences > 0);
    assert(forest.edges.size() == expected_edges);
    std::size_t defect_edges = 0;
    for (const auto& [_, count] : forest.edge_count_by_defect) defect_edges += count;
    assert(defect_edges == forest.edges.size());
    std::cout << " forest_scc=" << holonomy.components.size()
              << " twisted_scc=" << holonomy.nontrivial_holonomy_components
              << " holonomy_gcd=" << holonomy.components.front().holonomy_gcd
              << " branching_labels=" << regularity.branching_source_labels
              << " max_mult=" << regularity.max_occurrences_for_source_target_label
              << " composite_missing=" << composition.missing_pairs
              << " zero_pairs@10=" << closure.zero_net_pairs
              << "/" << forest.role_count * forest.role_count
              << " integer_window@2=" << closure.integer_window_pairs << "/"
              << forest.role_count * forest.role_count * 5;
}

void inspect_forest(const std::string& name, std::vector<long long> low_first,
                    std::size_t word_cap = 10, std::size_t radius = 3) {
    auto R = ring_from_low_first(low_first);
    auto beta_I = isolate_beta(R);
    const auto forest = ravel::proof::derive_canonical_parent_role_catalogue(R, beta_I);
    const auto holonomy = ravel::proof::derive_parent_role_holonomy(forest);
    const auto closure = ravel::proof::derive_parent_role_word_closure(forest, word_cap, radius);
    const auto profile = ravel::proof::derive_parent_role_window_profile(
        forest, word_cap, std::max<std::size_t>(10, radius));
    const auto cycles = ravel::proof::derive_parent_role_unit_cycles(forest, word_cap);
    const auto schema = ravel::proof::derive_canonical_unit_cycle_schema(forest);
    const auto transport = ravel::proof::derive_parent_role_integer_transport(
        forest, word_cap, radius);
    const auto scheme = ravel::proof::derive_parent_role_integer_scheme(
        forest, word_cap, word_cap);
    assert(forest.proved && holonomy.proved && closure.proved);
    assert(profile.proved);
    assert(cycles.proved && cycles.positive_cycle_found && cycles.negative_cycle_found);
    assert(cycles.positive_roles.size() == cycles.positive_defects.size() + 1);
    assert(cycles.negative_roles.size() == cycles.negative_defects.size() + 1);
    assert(schema.proved && schema.canonical_shape && schema.digit_driver_prefix_rule);
    assert(transport.proved);
    assert(scheme.proved && scheme.arbitrary_integer_displacement);
    std::cout << name << ": alphabet=" << forest.alphabet_size
              << " roles=" << forest.role_count
              << " defects=" << forest.defects.size()
              << " scc=" << holonomy.components.size()
              << " gcd=" << holonomy.components.front().holonomy_gcd
              << " zero_pairs@" << word_cap << "=" << closure.zero_net_pairs << "/"
              << forest.role_count * forest.role_count
              << " max_zero_word=" << closure.maximum_zero_net_word_length
              << " integer_window@" << radius << "=" << closure.integer_window_pairs << "/"
              << forest.role_count * forest.role_count * (2 * radius + 1) << "\n";
    std::cout << "  window_profile largest_complete_radius="
              << profile.largest_complete_radius << " missing@profile_max="
              << profile.missing_by_radius.back()
              << " unit_cycle_lengths=" << cycles.positive_defects.size()
              << "/" << cycles.negative_defects.size()
              << " unit_cycle_defects=";
    for (const auto d : cycles.positive_defects) std::cout << d << ",";
    std::cout << "/";
    for (const auto d : cycles.negative_defects) std::cout << d << ",";
    std::cout << " roles=";
    for (const auto role : cycles.positive_roles) std::cout << role << ",";
    std::cout << "/";
    for (const auto role : cycles.negative_roles) std::cout << role << ",";
    std::cout << " transport=" << transport.transported_words
              << " max_transport_word=" << transport.maximum_transport_word_length
              << " arbitrary_integer_scheme=" << scheme.arbitrary_integer_displacement
              << " canonical_unit_schema=" << schema.canonical_shape
              << "\n";
}

int main() {
    // Terminating, no cyclotomic padding.
    run("golden ratio", {-1, -1}, 3);
    run("theta4 supergolden", {-1, 0, -1}, 3);
    // Terminating, WITH cyclotomic padding (alphabet > degree).
    run("theta1 plastic", {0, -1, -1}, 3);
    run("theta3", {-1, -1, 1, 0, -1}, 3);
    // Terminating, non-unit, no genuine extra generator.
    run("x^2-2x-2", {-2, -2}, 3);
    // Terminating, non-unit, GENUINE extra generators.
    run("x^3-2x^2-2", {-2, 0, -2}, 5);
    run("x^3-x^2-2x-2", {-1, -2, -2}, 5);
    // Eventually periodic (genuine preperiod), unit, GENUINE extra generators.
    run("x^3-2x^2-x+1", {-2, -1, 1}, 5);
    // Eventually periodic, no extra generators (max digit is 1 throughout).
    run("x^3-x^2-2x+1", {-1, -2, 1}, 3);

    // Higher canonical families: inspect the finite forest without launching
    // the much larger Property-(F) node search.
    inspect_forest("theta7 forest", {-1, -1, 0, 0, 1, 0, -1}, 30, 3);
    inspect_forest("theta8 forest", {-2, 1, 0, -1, 1, -1}, 30, 3);
    inspect_forest("theta9 forest", {-1, 0, -1, 0, -1}, 30, 3);
    inspect_forest("theta10 forest", {-1, -1, 0, 0, 0, 1, 0, -1}, 100, 20);

    std::cout << "\nALL PASS: one call, no case split, correct in every case -- terminating "
                 "or eventually periodic, padded or not, unit or not.\n";
}
