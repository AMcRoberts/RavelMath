#include <cstdio>
#include <set>

#include "ravel/class_ii_boundary_family.hpp"

using namespace ravel;

int main() {
    bool ok = true;
    std::set<SNode<3>> seen;

    const auto contact = class_ii_contact_set();
    const auto d_cont_faces = class_ii_d_cont_face_candidates();
    const auto d_cont = class_ii_d_cont_set();
    const auto backward_one =
        class_ii_pre_contact_first_backward_layer();
    const auto backward_two =
        class_ii_pre_contact_second_backward_layer();
    const auto pre_contact = class_ii_pre_contact_set();
    const auto contact_exclusions = class_ii_contact_red_exclusions();
    const auto signed_contact = build_signed_contact_set<3>(contact);
    const auto stable_base = class_ii_stable_base();
    ok = ok
        && contact.size() == 14
        && d_cont_faces.size() == 33
        && d_cont.size() == 9
        && backward_one.size() == 6
        && backward_two.size() == 1
        && class_ii_pre_contact_backward_categories().size() == 51
        && class_ii_pre_contact_backward_category_ranges().size() == 51
        && pre_contact.size() == 16
        && contact_exclusions.size() == 2
        && signed_contact.size() == 28
        && stable_base.size() == 68;
    for (const auto& seed : d_cont)
        ok = ok && pre_contact.count(seed) == 1;
    for (const auto& node : backward_one)
        ok = ok
            && pre_contact.count(node) == 1
            && d_cont.count(node) == 0;
    for (const auto& node : backward_two)
        ok = ok
            && pre_contact.count(node) == 1
            && d_cont.count(node) == 0
            && backward_one.count(node) == 0;
    for (const auto& excluded : contact_exclusions)
        ok = ok
            && pre_contact.count(excluded) == 1
            && contact.count(to_simple<3>(excluded)) == 0;
    for (const auto& node : pre_contact) {
        const bool excluded = contact_exclusions.count(node) == 1;
        ok = ok
            && (excluded
                || contact.count(to_simple<3>(node)) == 1);
    }

    // Cheap structural regression protection for the closed formulas.
    // The universal distinctness theorem lives in Lean; this checks that
    // later C++ edits continue to represent the same object.
    for (std::size_t round = 4; round <= 128; ++round) {
        const auto shell = class_ii_interior_shell(round);
        ok = ok && shell.size() == 20;
        for (const auto& node : shell) {
            ok = ok && seen.insert(node).second;
        }

        const auto terminal = class_ii_terminal_shell(round);
        ok = ok && terminal.size() == 20;
    }

    for (std::size_t a = 2; a <= 128; ++a) {
        for (const auto& [category, range] :
             class_ii_pre_contact_backward_category_ranges()) {
            (void)category;
            ok = ok
                && range.minimum_intercept
                    + range.minimum_slope * static_cast<long long>(a)
                <= range.maximum_intercept
                    + range.maximum_slope * static_cast<long long>(a);
        }
        ok = ok
            && class_ii_pre_contact_backward_witnesses(a).size() == 7
            && class_ii_recurrent_shell_component(a, 1).size() == 2
            && class_ii_recurrent_shell_matrix(1)
                == std::vector<std::vector<long long>>{{0, 1}, {1, 0}};
        for (std::size_t k = 2; k < a; ++k) {
            const auto matrix = class_ii_recurrent_shell_matrix(k);
            ok = ok
                && class_ii_recurrent_shell_component(a, k).size() == 4
                && matrix.size() == 4
                && matrix[0][2] == static_cast<long long>(k - 1)
                && matrix[1][2] == 1
                && matrix[2][0] == static_cast<long long>(k - 1)
                && matrix[3][0] == 1;
        }
    }
    for (std::size_t a = 4; a <= 128; ++a) {
        ok = ok
            && class_ii_center_layer_candidate(a, 3) == stable_base;
        for (std::size_t round = 4; round < a; ++round) {
            ok = ok
                && class_ii_center_layer_candidate(a, round).size()
                    == 68 + 20 * (round - 3);
        }
        ok = ok
            && class_ii_center_layer_candidate(a, a).size()
                == 20 * a + 8
            && class_ii_center_layer_candidate(a, a + 1)
                == class_ii_center_layer_candidate(a, a);
    }

    std::printf(
        "class_ii_boundary_family_test: %s "
        "(9/33 seed table, 16=14+2 contact split, "
        "51 affine backward ranges, "
        "125 disjoint shells, closed SCC formulas through a=128)\n",
        ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
