// Symbolic Class-II tail grammar, independent of a realized Property-(F)
// graph. It composes the parameter-uniform prefix roles with the affine
// beta-inverse recurrence and checks that the explicit spine has no alternate
// spine continuation or predecessor. Graph-level collar absorption remains a
// separate obligation.
#pragma once

#include <array>
#include <cstddef>

#include "adelic/property_f_class_ii_affine_tail.hpp"
#include "adelic/property_f_class_ii_prefix_role_grammar.hpp"

namespace adelic {

struct PropertyFClassIISymbolicTailGrammarCertificate {
    std::size_t parameter_a = 0;
    std::size_t spine_states = 0;
    std::size_t forward_candidates = 0;
    std::size_t forward_expected_hits = 0;
    std::size_t forward_internal_hits = 0;
    std::size_t forward_alternate_spine_hits = 0;
    std::size_t reverse_candidates = 0;
    std::size_t reverse_expected_hits = 0;
    std::size_t reverse_missing_targets = 0;
    std::size_t reverse_internal_hits = 0;
    std::size_t reverse_alternate_spine_hits = 0;
    bool parameter_domain = false;
    bool affine_valid = false;
    bool role_grammar_valid = false;
    bool forward_unique = false;
    bool reverse_unique = false;
    bool valid = false;
};

inline std::array<long long, 3>
property_f_class_ii_affine_tail_inverse_step(
        const std::array<long long, 3>& target,
        long long a, long long digit) {
    // Invert (c0,c1,c2) ->
    // (c1-(a+1)c0+j, c2-a*c0, c0).
    return {target[2],
            target[0] + (a + 1) * target[2] - digit,
            target[1] + a * target[2]};
}

inline std::size_t property_f_class_ii_spine_letter(std::size_t step) {
    return step == 0 ? 0 : (step == 1 ? 1 : 0);
}

inline PropertyFClassIISymbolicTailGrammarCertificate
derive_property_f_class_ii_symbolic_tail_grammar(std::size_t a) {
    PropertyFClassIISymbolicTailGrammarCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 4 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;
    const auto states = property_f_class_ii_rank_spine_coefficients(a);
    out.spine_states = states.size();
    out.affine_valid =
        derive_property_f_class_ii_affine_tail_certificate(a).valid;
    out.role_grammar_valid =
        derive_property_f_class_ii_prefix_role_grammar(a).valid;
    if (!out.affine_valid || !out.role_grammar_valid) return out;
    const auto aa = static_cast<long long>(a);

    out.forward_unique = true;
    for (std::size_t step = 0; step < states.size(); ++step) {
        const long long current_letter = static_cast<long long>(
            property_f_class_ii_spine_letter(step));
        const auto roles = property_f_class_ii_prefix_roles_for_target(
            a, current_letter);
        std::size_t expected_hits = 0;
        for (const auto& [parent, positions] : roles) {
            for (const auto position : positions) {
                ++out.forward_candidates;
                const auto candidate =
                    property_f_class_ii_affine_tail_step(
                        states[step], aa,
                        static_cast<long long>(position));
                std::size_t matched = states.size();
                for (std::size_t other = 0; other < states.size(); ++other) {
                    if (static_cast<long long>(parent) ==
                            static_cast<long long>(
                                property_f_class_ii_spine_letter(other)) &&
                        candidate == states[other]) {
                        matched = other;
                        break;
                    }
                }
                if (matched == states.size()) continue;
                if (step + 1 < states.size() && matched == step + 1)
                    ++expected_hits;
                else if (matched == step)
                    ++out.forward_internal_hits;
                else
                    ++out.forward_alternate_spine_hits;
            }
        }
        if (step + 1 < states.size()) {
            out.forward_expected_hits += expected_hits;
            if (expected_hits != 1) out.forward_unique = false;
        }
    }

    out.reverse_unique = true;
    for (std::size_t target_step = 0; target_step < states.size();
         ++target_step) {
        const long long target_letter = static_cast<long long>(
            property_f_class_ii_spine_letter(target_step));
        std::size_t expected_hits = 0;
        for (long long source_letter = 0; source_letter <= 2;
             ++source_letter) {
            const auto roles = property_f_class_ii_prefix_roles_for_target(
                a, source_letter);
            const auto parent_it = roles.find(target_letter);
            if (parent_it == roles.end()) continue;
            for (const auto position : parent_it->second) {
                ++out.reverse_candidates;
                const auto candidate =
                    property_f_class_ii_affine_tail_inverse_step(
                        states[target_step], aa,
                        static_cast<long long>(position));
                std::size_t matched = states.size();
                for (std::size_t other = 0; other < states.size(); ++other) {
                    if (source_letter == static_cast<long long>(
                            property_f_class_ii_spine_letter(other)) &&
                        candidate == states[other]) {
                        matched = other;
                        break;
                    }
                }
                if (matched == states.size()) continue;
                if (target_step > 0 && matched == target_step - 1)
                    ++expected_hits;
                else if (matched == target_step)
                    ++out.reverse_internal_hits;
                else
                    ++out.reverse_alternate_spine_hits;
            }
        }
        if (target_step > 0) {
            out.reverse_expected_hits += expected_hits;
            if (expected_hits != 1) {
                out.reverse_unique = false;
                ++out.reverse_missing_targets;
            }
        }
    }
    out.valid = out.parameter_domain && out.affine_valid &&
                out.role_grammar_valid && out.forward_unique &&
                out.reverse_unique && out.forward_alternate_spine_hits == 0 &&
                out.reverse_alternate_spine_hits == 0;
    return out;
}

}  // namespace adelic
