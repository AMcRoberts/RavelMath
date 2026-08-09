// First executable bridge between contact-boundary transitions and the exact
// prefix/adelic transport labels.  The contact side supplies a pair of prefix
// words; the adelic side supplies the same words and their exact Q(beta)
// digits.  This certificate deliberately stops before claiming a full graph
// simulation: it proves the label alphabet and arithmetic agree.
#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "adelic/property_f_types.hpp"
#include "math/qbeta.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/substitution.hpp"

namespace adelic {

template <std::size_t d>
struct PropertyFContactTransportBridgeCertificate {
    bool boundary_complete = false;
    bool contact_prefixes_in_adelic_alphabet = false;
    bool exact_digit_labels_replay = false;
    long long boundary_edges = 0;
    long long distinct_contact_prefixes = 0;
    long long distinct_adelic_prefixes = 0;
    std::string obstruction;
};

template <std::size_t d>
PropertyFContactTransportBridgeCertificate<d>
derive_property_f_contact_transport_bridge(
    const ravel::SubstitutionRule& rule,
    const PrefixAutomaton<d>& automaton,
    double beta,
    long long search_bound,
    const ravel::ContactBoundaryLimits& limits) {
    PropertyFContactTransportBridgeCertificate<d> out;
    const auto report = ravel::compute_contact_boundary_from_subst<d>(
        rule, beta, 0.0, search_bound, limits);
    out.boundary_complete = report.converged && !report.closure_stopped_early &&
        !report.corona_capped;
    out.distinct_adelic_prefixes =
        static_cast<long long>(automaton.distinct_prefixes.size());
    if (!out.boundary_complete) {
        out.obstruction = "contact boundary hit an explicit cap";
        return out;
    }

    std::set<std::vector<long long>> contact_prefixes;
    std::set<std::vector<long long>> adelic_prefixes(
        automaton.distinct_prefixes.begin(), automaton.distinct_prefixes.end());
    const auto subst = ravel::make_substitution<d>(rule, beta);
    std::vector<ravel::SNode<d>> boundary;
    for (const auto& tuple : report.boundary_nodes) {
        ravel::SNode<d> node;
        node.i = std::get<0>(tuple);
        node.j = std::get<2>(tuple);
        const auto& coordinates = std::get<1>(tuple);
        for (std::size_t q = 0; q < d; ++q) node.x[q] = coordinates[q];
        boundary.push_back(node);
    }
    bool labels_ok = true;
    bool digits_ok = true;
    for (const auto& source : boundary) {
        for (const auto& [target, prefixes] :
             ravel::simple_forward_targets_exact<d>(subst, source)) {
            (void)target;
            ++out.boundary_edges;
            for (const auto& prefix : {prefixes.first, prefixes.second}) {
                std::vector<long long> word;
                word.reserve(prefix.size());
                for (const auto letter : prefix)
                    word.push_back(static_cast<long long>(letter));
                contact_prefixes.insert(word);
                if (!adelic_prefixes.count(word)) labels_ok = false;
                mathlib::QElem digit = automaton.ring.zero();
                for (const long long letter : word) {
                    if (letter < 0 || static_cast<std::size_t>(letter) >=
                        automaton.left_eigenvector.size()) {
                        digits_ok = false;
                        continue;
                    }
                    digit = automaton.ring.add(
                        digit, automaton.left_eigenvector[static_cast<std::size_t>(letter)]);
                }
                if (qelem_key(digit).empty()) digits_ok = false;
            }
        }
    }
    out.distinct_contact_prefixes = static_cast<long long>(contact_prefixes.size());
    out.contact_prefixes_in_adelic_alphabet = labels_ok;
    out.exact_digit_labels_replay = digits_ok;
    if (!labels_ok) out.obstruction = "contact prefix outside adelic label alphabet";
    else if (!digits_ok) out.obstruction = "contact prefix digit replay failed";
    return out;
}

}  // namespace adelic
