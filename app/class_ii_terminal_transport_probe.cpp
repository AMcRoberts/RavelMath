#include <cstdio>
#include <string>

#include "ravel/class_ii_neighbor2_pruning.hpp"

int main(int argc, char** argv) {
    using namespace ravel;
    std::printf("Class-II neighbor-2 terminal inherited transport\n");
    std::fflush(stdout);
    auto fixed_sources =
        class_ii_neighbor2_interior_pruning_rank1_seed();
    for (const auto& node :
         class_ii_neighbor2_interior_pruning_frontier(4))
        fixed_sources.erase(node);
    std::printf("fixed grammar prepared: %zu sources\n",
                fixed_sources.size());
    std::fflush(stdout);
    const auto target_forms =
        class_ii_neighbor2_terminal_new_survivor_target_forms();
    {
        std::set<SNode<3>> at_nine;
        for (const auto& form : target_forms)
            at_nine.insert(
                class_ii_evaluate_affine_node_form(form, 9));
        const auto decomposed =
            class_ii_neighbor2_terminal_new_survivor_targets_decomposed(9);
        std::printf(
            "decomposed target grammar a=9 affine=%zu literal=%zu exact=%d\n",
            at_nine.size(), decomposed.size(),
            at_nine == decomposed ? 1 : 0);
    }
    for (long long a : {7LL, 8LL, 9LL}) {
        auto terminal_extension =
            class_ii_neighbor2_center_extension(a);
        const auto inherited_extension =
            class_ii_neighbor2_interior_extension_states(
                static_cast<std::size_t>(a - 2));
        for (const auto& node : inherited_extension)
            terminal_extension.erase(node);
        std::set<SNode<3>> moving_targets;
        for (const auto& form :
             class_ii_neighbor2_terminal_target_forms()) {
            bool moving = false;
            for (const auto& coordinate : form.x)
                moving = moving || coordinate.slope != 0;
            if (moving)
                moving_targets.insert(
                    class_ii_evaluate_affine_node_form(form, a));
        }
        std::printf(
            "extension difference a=%lld roles=%zu affine=%zu exact=%d\n",
            a, terminal_extension.size(), moving_targets.size(),
            terminal_extension == moving_targets ? 1 : 0);
    }
    if (argc == 2 && std::string(argv[1]) == "--validate-grammar") {
        for (long long a : {7LL, 8LL}) {
            std::set<SNode<3>> affine;
            for (const auto& form : target_forms)
                affine.insert(
                    class_ii_evaluate_affine_node_form(form, a));
            const auto literal =
                class_ii_neighbor2_terminal_new_survivor_targets(a);
            std::printf(
                "target grammar a=%lld affine=%zu literal=%zu exact=%d\n",
                a, affine.size(), literal.size(),
                affine == literal ? 1 : 0);
            for (const auto& node : literal)
                if (!affine.count(node))
                    std::printf(
                        "  literal-only (%lld; [%lld,%lld,%lld]; %lld)\n",
                        node.i, node.x[0], node.x[1], node.x[2], node.j);
            for (const auto& node : affine)
                if (!literal.count(node))
                    std::printf(
                        "  affine-only (%lld; [%lld,%lld,%lld]; %lld)\n",
                        node.i, node.x[0], node.x[1], node.x[2], node.j);
            std::fflush(stdout);
        }
    }
    if (argc == 2 && std::string(argv[1]) == "--red-audit") {
        constexpr long long a = 7;
        auto nodes = class_ii_stable_base();
        for (long long q = 4; q < a; ++q) {
            const auto shell =
                class_ii_interior_shell(static_cast<std::size_t>(q));
            nodes.insert(shell.begin(), shell.end());
        }
        const auto terminal_shell_nodes = class_ii_terminal_shell(a);
        nodes.insert(
            terminal_shell_nodes.begin(), terminal_shell_nodes.end());
        const auto extension = class_ii_neighbor2_center_extension(a);
        nodes.insert(extension.begin(), extension.end());
        const auto claimed =
            class_ii_neighbor2_terminal_pruning_ranks(a);
        for (const auto& rank : claimed)
            nodes.insert(rank.begin(), rank.end());
        std::vector<std::tuple<SNode<3>, SNode<3>,
                               std::vector<long long>,
                               std::vector<long long>>> edges;
        for (const auto& source : nodes)
            for (const auto& target : nodes)
                if (class_ii_neighbor_transition_weight(
                        2, a, source, target) != 0)
                    edges.emplace_back(
                        source, target,
                        std::vector<long long>{},
                        std::vector<long long>{});
        const auto actual = red_trace<3>(nodes, edges);
        std::printf(
            "red audit a=7 nodes=%zu edges=%zu survivors=%zu ranks=%zu\n",
            nodes.size(), edges.size(), actual.survivors.size(),
            actual.pruning_ranks.size());
        const std::size_t ranks =
            std::max(actual.pruning_ranks.size(), claimed.size());
        for (std::size_t rank = 0; rank < ranks; ++rank) {
            const std::set<SNode<3>> empty;
            const auto& observed = rank < actual.pruning_ranks.size()
                ? actual.pruning_ranks[rank] : empty;
            const auto& expected = rank < claimed.size()
                ? claimed[rank] : empty;
            std::size_t missing = 0;
            std::size_t extra = 0;
            for (const auto& node : expected)
                missing += !observed.count(node);
            for (const auto& node : observed)
                extra += !expected.count(node);
            std::printf(
                "  rank %zu observed=%zu claimed=%zu "
                "claimed-not-observed=%zu observed-not-claimed=%zu\n",
                rank + 1, observed.size(), expected.size(),
                missing, extra);
        }
        std::fflush(stdout);
    }
    std::printf(
        "input grammar fixed=%zu indexed-cell-roles=%zu targets=%zu\n",
        fixed_sources.size(),
        class_ii_neighbor2_pruning_cell_forms().size(),
        target_forms.size());
    std::fflush(stdout);

    const auto fixed =
        class_ii_neighbor2_terminal_inherited_fixed_to_new_targets();
    std::printf(
        "fixed sources=%zu targets=%zu pairs=%zu branches=%zu "
        "unresolved=%zu exact=%d\n",
        fixed.source_count, fixed.target_count, fixed.pair_count,
        fixed.prefix_branches, fixed.unresolved_branches,
        fixed.exact ? 1 : 0);
    std::fflush(stdout);

    const auto cells =
        class_ii_neighbor2_terminal_inherited_cells_to_new_targets();
    std::printf(
        "indexed cells=%zu targets=%zu pairs=%zu branches=%zu "
        "unresolved=%zu exact=%d\n",
        cells.source_count, cells.target_count, cells.pair_count,
        cells.prefix_branches, cells.unresolved_branches,
        cells.exact ? 1 : 0);
    for (const auto& [source, target] : cells.unresolved_pairs) {
        const auto print_form = [](const char* label,
                                   const auto& form) {
            std::printf(
                "  %s (%lld; [%lld%+lldq,%lld%+lldq,%lld%+lldq]; "
                "%lld)\n",
                label, form.i,
                form.x[0].intercept, form.x[0].slope,
                form.x[1].intercept, form.x[1].slope,
                form.x[2].intercept, form.x[2].slope,
                form.j);
        };
        print_form("source", source);
        print_form("target", target);
        for (long long a = 7; a <= 16; ++a) {
            for (long long q = 5; q <= a - 2; ++q) {
                const auto source_node =
                    class_ii_evaluate_affine_node_form(source, q);
                const auto target_node =
                    class_ii_evaluate_affine_node_form(target, a);
                const auto weight =
                    class_ii_neighbor_transition_weight(
                        2, a, source_node, target_node);
                if (weight != 0)
                    std::printf("    finite edge a=%lld q=%lld w=%lld\n",
                                a, q, weight);
            }
        }
    }

    const auto endpoint =
        class_ii_neighbor2_terminal_inherited_rank1_endpoint_to_new_targets();
    std::set<SNode<3>> endpoint_at_seven;
    for (const auto& form :
         class_ii_neighbor2_terminal_inherited_rank1_endpoint_forms())
        endpoint_at_seven.insert(
            class_ii_evaluate_affine_node_form(form, 7));
    std::size_t removed_endpoint_overlap = 0;
    for (const auto& node :
         class_ii_neighbor2_terminal_rank1_removed(7))
        removed_endpoint_overlap += endpoint_at_seven.count(node);
    std::printf(
        "edited endpoint sources=%zu targets=%zu pairs=%zu branches=%zu "
        "unresolved=%zu removed-overlap@7=%zu exact=%d\n",
        endpoint.source_count, endpoint.target_count, endpoint.pair_count,
        endpoint.prefix_branches, endpoint.unresolved_branches,
        removed_endpoint_overlap,
        endpoint.exact ? 1 : 0);
    std::set<std::pair<SNode<3>, SNode<3>>> endpoint_pairs;
    for (const auto& [source, target] : endpoint.unresolved_pairs)
        endpoint_pairs.insert({
            class_ii_evaluate_affine_node_form(source, 7),
            class_ii_evaluate_affine_node_form(target, 7)});
    std::size_t nonzero_endpoint_pairs = 0;
    for (const auto& [source, target] : endpoint_pairs)
        nonzero_endpoint_pairs +=
            class_ii_neighbor_transition_weight(
                2, 7, source, target) != 0;
    std::printf(
        "edited endpoint unresolved pairs=%zu nonzero@7=%zu\n",
        endpoint_pairs.size(), nonzero_endpoint_pairs);

    const auto added_rank2 =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_neighbor2_terminal_rank2_added_forms(),
            target_forms, 0, 0, 7);
    std::printf("added rank2 completed\n");
    std::fflush(stdout);
    const auto post_rank2_at_seven =
        class_ii_neighbor2_terminal_new_post_rank2_targets_decomposed(7);
    std::printf("post-rank2 a=7 roles=%zu\n",
                post_rank2_at_seven.size());
    std::fflush(stdout);
    const auto post_rank2_at_eight =
        class_ii_neighbor2_terminal_new_post_rank2_targets_decomposed(8);
    std::printf("post-rank2 a=8 roles=%zu\n",
                post_rank2_at_eight.size());
    std::fflush(stdout);
    const auto post_rank2_targets =
        class_ii_neighbor2_terminal_new_post_rank2_target_forms();
    std::printf("post-rank2 target grammar roles=%zu\n",
                post_rank2_targets.size());
    std::fflush(stdout);
    const auto added_rank3 =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_neighbor2_terminal_rank3_added_forms(),
            post_rank2_targets, 0, 0, 7);
    auto inherited_fixed_post_rank1 = class_ii_stable_base();
    const auto fixed_extension =
        class_ii_neighbor2_fixed_extension_states();
    const auto stable_rank2 =
        class_ii_neighbor2_interior_pruning_rank2();
    const auto stable_rank3 =
        class_ii_neighbor2_interior_pruning_rank3();
    inherited_fixed_post_rank1.insert(
        fixed_extension.begin(), fixed_extension.end());
    inherited_fixed_post_rank1.insert(
        stable_rank2.begin(), stable_rank2.end());
    inherited_fixed_post_rank1.insert(
        stable_rank3.begin(), stable_rank3.end());
    for (const auto& node : fixed_sources)
        inherited_fixed_post_rank1.erase(node);
    auto inherited_fixed_post_rank2 = inherited_fixed_post_rank1;
    for (const auto& node : stable_rank2)
        inherited_fixed_post_rank2.erase(node);
    const std::vector<ClassIINodeAffineParameterForm> inherited_tip = {
        {2, {{{2, -1}, {-2, 1}, {-1, 0}}}, 0},
    };
    const auto append_forms = [](auto left, const auto& right) {
        left.insert(left.end(), right.begin(), right.end());
        return left;
    };
    const auto inherited_rank2_bounded =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_neighbor2_terminal_rank2_added_forms(),
            append_forms(
                class_ii_constant_node_forms(inherited_fixed_post_rank1),
                inherited_tip),
            0, 0, 7);
    const auto inherited_rank3_bounded =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_neighbor2_terminal_rank3_added_forms(),
            append_forms(
                class_ii_constant_node_forms(inherited_fixed_post_rank2),
                inherited_tip),
            0, 0, 7);
    const std::vector<ClassIIPruningConeForm> inherited_shell_domain = {
        {-7, 1, 0, 0},
        {0, -1, 1, 0}, {0, 1, -1, 0},
        {-4, 0, 0, 1}, {-2, 1, 0, -1},
    };
    const auto inherited_rank2_shells =
        class_ii_neighbor2_affine_pair_exclusion(
            class_ii_neighbor2_terminal_rank2_added_forms(),
            class_ii_center_interior_shell_forms(),
            false, inherited_shell_domain);
    const auto inherited_rank3_shells =
        class_ii_neighbor2_affine_pair_exclusion(
            class_ii_neighbor2_terminal_rank3_added_forms(),
            class_ii_center_interior_shell_forms(),
            false, inherited_shell_domain);
    const auto stable_rank2_to_new =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_constant_node_forms(stable_rank2),
            target_forms, 0, 0, 7);
    const auto stable_rank3_to_new =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_constant_node_forms(stable_rank3),
            post_rank2_targets, 0, 0, 7);
    std::printf(
        "added later ranks to new targets rank2=%zu/%zu rank3=%zu/%zu "
        "unresolved=%zu+%zu exact=%d/%d\n",
        added_rank2.source_count, added_rank2.prefix_branches,
        added_rank3.source_count, added_rank3.prefix_branches,
        added_rank2.unresolved_branches,
        added_rank3.unresolved_branches,
        added_rank2.exact ? 1 : 0, added_rank3.exact ? 1 : 0);
    std::printf(
        "added later ranks to inherited targets bounded=%zu+%zu "
        "shells=%zu+%zu unresolved=%zu+%zu+%zu+%zu exact=%d/%d/%d/%d\n",
        inherited_rank2_bounded.prefix_branches,
        inherited_rank3_bounded.prefix_branches,
        inherited_rank2_shells.prefix_branches,
        inherited_rank3_shells.prefix_branches,
        inherited_rank2_bounded.unresolved_branches,
        inherited_rank3_bounded.unresolved_branches,
        inherited_rank2_shells.unresolved_branches,
        inherited_rank3_shells.unresolved_branches,
        inherited_rank2_bounded.exact ? 1 : 0,
        inherited_rank3_bounded.exact ? 1 : 0,
        inherited_rank2_shells.exact ? 1 : 0,
        inherited_rank3_shells.exact ? 1 : 0);
    std::printf(
        "stable later ranks to new targets rank2=%zu rank3=%zu "
        "unresolved=%zu+%zu exact=%d/%d\n",
        stable_rank2_to_new.prefix_branches,
        stable_rank3_to_new.prefix_branches,
        stable_rank2_to_new.unresolved_branches,
        stable_rank3_to_new.unresolved_branches,
        stable_rank2_to_new.exact ? 1 : 0,
        stable_rank3_to_new.exact ? 1 : 0);
    const auto predecessor_forms = [](const auto& source_forms,
                                      std::size_t target_rank) {
        std::vector<ClassIIAffineRedEdge> result;
        const auto ranks7 =
            class_ii_neighbor2_terminal_pruning_ranks(7);
        const auto ranks8 =
            class_ii_neighbor2_terminal_pruning_ranks(8);
        for (const auto& source_form : source_forms) {
            const auto source7 =
                class_ii_evaluate_affine_node_form(source_form, 7);
            const auto source8 =
                class_ii_evaluate_affine_node_form(source_form, 8);
            bool found = false;
            for (const auto& target7 : ranks7[target_rank]) {
                if (class_ii_neighbor_transition_weight(
                        2, 7, source7, target7) != 1)
                    continue;
                for (const auto& target8 : ranks8[target_rank]) {
                    if (target7.i != target8.i
                            || target7.j != target8.j
                            || target7.x[2] != target8.x[2]
                            || target7.x[0] + target7.x[1]
                                != target8.x[0] + target8.x[1]
                            || std::abs(target8.x[0] - target7.x[0]) > 1
                            || std::abs(target8.x[1] - target7.x[1]) > 1
                            || class_ii_neighbor_transition_weight(
                                2, 8, source8, target8) != 1)
                        continue;
                    ClassIINodeAffineParameterForm target_form;
                    target_form.i = target7.i;
                    target_form.j = target7.j;
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        const long long slope =
                            target8.x[coordinate]
                            - target7.x[coordinate];
                        target_form.x[coordinate] = {
                            target7.x[coordinate] - 7 * slope,
                            slope,
                        };
                    }
                    const ClassIIAffineRedEdge edge{
                        source_form, target_form};
                    if (class_ii_neighbor2_affine_edge_weight_one(
                            edge, 7, 0, 0)) {
                        result.push_back(edge);
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }
        return result;
    };
    const auto rank2_predecessors = predecessor_forms(
        class_ii_neighbor2_terminal_rank2_added_forms(), 0);
    const auto rank3_predecessors = predecessor_forms(
        class_ii_neighbor2_terminal_rank3_added_forms(), 1);
    std::printf(
        "added later-rank predecessors rank2=%zu/22 rank3=%zu/2\n",
        rank2_predecessors.size(), rank3_predecessors.size());

    auto terminal_targets =
        class_ii_neighbor2_stable_affine_target_forms();
    const auto terminal_new_targets =
        class_ii_neighbor2_terminal_new_pre_red_target_forms();
    terminal_targets.insert(
        terminal_targets.end(),
        terminal_new_targets.begin(), terminal_new_targets.end());
    std::set<ClassIINodeAffineRoleKey> terminal_target_keys;
    for (const auto& form : terminal_targets)
        terminal_target_keys.insert(
            class_ii_affine_node_role_key(form));
    std::set<ClassIINodeAffineRoleKey> fixed_target_keys;
    for (const auto& form : class_ii_constant_node_forms(
             class_ii_neighbor2_stable_fixed_target_nodes()))
        fixed_target_keys.insert(class_ii_affine_node_role_key(form));
    auto hops = class_ii_neighbor2_signed_contact_set();
    for (long long color = 0; color < 3; ++color)
        hops.insert({color, {0, 0, 0}, color});
    std::map<ClassIINodeAffineRoleKey,
             ClassIINodeAffineParameterForm> terminal_compositions;
    auto fixed_terminal_sources = class_ii_center_layer_candidate(7, 4);
    const auto fixed_terminal_extension =
        class_ii_neighbor2_layer_extension(7, 4);
    fixed_terminal_sources.insert(
        fixed_terminal_extension.begin(), fixed_terminal_extension.end());
    auto full_terminal_sources =
        class_ii_constant_node_forms(fixed_terminal_sources);
    auto endpoint_shell_sources =
        class_ii_center_interior_shell_forms();
    for (auto& form : endpoint_shell_sources)
        for (auto& coordinate : form.x)
            coordinate.intercept -= coordinate.slope;
    full_terminal_sources.insert(
        full_terminal_sources.end(),
        endpoint_shell_sources.begin(), endpoint_shell_sources.end());
    full_terminal_sources.push_back({
        2, {{{1, -1}, {-1, 1}, {-1, 0}}}, 0});
    for (auto form : class_ii_neighbor2_penultimate_transfer_forms()) {
        for (auto& coordinate : form.x)
            coordinate.intercept -= coordinate.slope;
        full_terminal_sources.push_back(form);
    }
    for (const auto& source : full_terminal_sources) {
        for (const auto& hop : hops) {
            if (source.j != hop.i) continue;
            auto form = source;
            form.j = hop.j;
            for (std::size_t coordinate = 0;
                 coordinate < 3; ++coordinate)
                form.x[coordinate].intercept += hop.x[coordinate];
            bool zero = form.i == form.j;
            for (const auto& coordinate : form.x)
                zero = zero && coordinate.intercept == 0
                    && coordinate.slope == 0;
            if (zero)
                continue;
            terminal_compositions[
                class_ii_affine_node_role_key(form)] = form;
        }
    }
    std::size_t terminal_matched = 0;
    std::size_t terminal_shifted = 0;
    std::size_t terminal_invalid = 0;
    std::size_t terminal_unresolved = 0;
    for (const auto& [key, form] : terminal_compositions) {
        if (terminal_target_keys.count(key)
                || fixed_target_keys.count(key)
                || class_ii_neighbor2_constant_instance_of_affine_role(
                    form, terminal_targets)) {
            ++terminal_matched;
            continue;
        }
        long long shift = 0;
        ClassIINodeAffineParameterForm matched;
        if (class_ii_neighbor2_shifted_affine_role(
                form, terminal_targets, shift, matched)
                && 7 + shift >= 4) {
            ++terminal_shifted;
            continue;
        }
        if (class_ii_neighbor2_affine_form_terminal_invalid(form))
            ++terminal_invalid;
        else {
            ++terminal_unresolved;
            std::printf(
                "  unresolved terminal form (%lld; "
                "[%lld%+llda,%lld%+llda,%lld%+llda]; %lld) "
                "valid=%d\n",
                form.i,
                form.x[0].intercept, form.x[0].slope,
                form.x[1].intercept, form.x[1].slope,
                form.x[2].intercept, form.x[2].slope,
                form.j,
                class_ii_neighbor2_affine_form_terminal_valid(form)
                    ? 1 : 0);
        }
    }
    std::size_t missing_affine_targets = 0;
    for (const auto& form : terminal_targets) {
        const auto key = class_ii_affine_node_role_key(form);
        if (!terminal_compositions.count(key)) {
            ++missing_affine_targets;
            std::printf(
                "  missing terminal target (%lld; "
                "[%lld%+llda,%lld%+llda,%lld%+llda]; %lld)\n",
                form.i,
                form.x[0].intercept, form.x[0].slope,
                form.x[1].intercept, form.x[1].slope,
                form.x[2].intercept, form.x[2].slope,
                form.j);
        }
    }
    std::size_t missing_fixed_targets = 0;
    for (const auto& key : fixed_target_keys)
        missing_fixed_targets += !terminal_compositions.count(key);
    std::size_t invalid_new_targets = 0;
    for (const auto& form : terminal_new_targets) {
        if (!class_ii_neighbor2_affine_form_terminal_valid(form)) {
            ++invalid_new_targets;
            std::printf(
                "  unproved terminal target validity (%lld; "
                "[%lld%+llda,%lld%+llda,%lld%+llda]; %lld) "
                "invalid=%d\n",
                form.i,
                form.x[0].intercept, form.x[0].slope,
                form.x[1].intercept, form.x[1].slope,
                form.x[2].intercept, form.x[2].slope,
                form.j,
                class_ii_neighbor2_affine_form_terminal_invalid(form)
                    ? 1 : 0);
        }
    }
    std::printf(
        "terminal endpoint compositions=%zu targets=%zu+%zu "
        "sources=%zu matched=%zu shifted=%zu invalid=%zu unresolved=%zu "
        "missing=%zu+%zu invalid-targets=%zu\n",
        terminal_compositions.size(),
        terminal_targets.size(), fixed_target_keys.size(),
        full_terminal_sources.size(),
        terminal_matched, terminal_shifted,
        terminal_invalid, terminal_unresolved,
        missing_affine_targets, missing_fixed_targets,
        invalid_new_targets);

    auto repeated_sources =
        class_ii_constant_node_forms(fixed_terminal_sources);
    const auto new_final_survivors =
        class_ii_neighbor2_terminal_new_final_survivor_forms();
    repeated_sources.insert(
        repeated_sources.end(),
        new_final_survivors.begin(), new_final_survivors.end());
    auto repeated_targets =
        class_ii_neighbor2_stable_affine_target_forms();
    const auto fixed_new_targets =
        class_ii_neighbor2_fixed_new_pre_red_target_forms();
    repeated_targets.insert(
        repeated_targets.end(),
        fixed_new_targets.begin(), fixed_new_targets.end());
    std::set<ClassIINodeAffineRoleKey> repeated_target_keys;
    for (const auto& form : repeated_targets)
        repeated_target_keys.insert(class_ii_affine_node_role_key(form));
    std::map<ClassIINodeAffineRoleKey,
             ClassIINodeAffineParameterForm> repeated_compositions;
    for (const auto& source : repeated_sources) {
        for (const auto& hop : hops) {
            if (source.j != hop.i) continue;
            auto form = source;
            form.j = hop.j;
            for (std::size_t coordinate = 0;
                 coordinate < 3; ++coordinate)
                form.x[coordinate].intercept += hop.x[coordinate];
            bool zero = form.i == form.j;
            for (const auto& coordinate : form.x)
                zero = zero && coordinate.intercept == 0
                    && coordinate.slope == 0;
            if (!zero)
                repeated_compositions[
                    class_ii_affine_node_role_key(form)] = form;
        }
    }
    std::size_t repeated_matched = 0;
    std::size_t repeated_shifted = 0;
    std::size_t repeated_invalid = 0;
    std::size_t repeated_unresolved = 0;
    for (const auto& [key, form] : repeated_compositions) {
        if (repeated_target_keys.count(key)
                || fixed_target_keys.count(key)
                || class_ii_neighbor2_constant_instance_of_affine_role(
                    form, repeated_targets)) {
            ++repeated_matched;
            continue;
        }
        long long shift = 0;
        ClassIINodeAffineParameterForm matched_form;
        if (class_ii_neighbor2_shifted_affine_role(
                form, repeated_targets, shift, matched_form)
                && 7 + shift >= 4) {
            ++repeated_shifted;
            continue;
        }
        if (class_ii_neighbor2_affine_form_terminal_invalid(form))
            ++repeated_invalid;
        else
            ++repeated_unresolved;
    }
    std::size_t repeated_missing = 0;
    for (const auto& key : repeated_target_keys)
        repeated_missing += !repeated_compositions.count(key);
    std::size_t repeated_invalid_targets = 0;
    for (const auto& form : fixed_new_targets) {
        if (!class_ii_neighbor2_affine_form_terminal_valid(form)) {
            ++repeated_invalid_targets;
            std::printf(
                "  unproved repeated target validity (%lld; "
                "[%lld%+llda,%lld%+llda,%lld%+llda]; %lld) "
                "invalid=%d\n",
                form.i,
                form.x[0].intercept, form.x[0].slope,
                form.x[1].intercept, form.x[1].slope,
                form.x[2].intercept, form.x[2].slope,
                form.j,
                class_ii_neighbor2_affine_form_terminal_invalid(form)
                    ? 1 : 0);
        }
    }
    std::printf(
        "repeated endpoint sources=%zu compositions=%zu targets=%zu+%zu "
        "classified=%zu+%zu+%zu unresolved=%zu missing=%zu "
        "invalid-targets=%zu\n",
        repeated_sources.size(), repeated_compositions.size(),
        repeated_targets.size(), fixed_target_keys.size(),
        repeated_matched, repeated_shifted, repeated_invalid,
        repeated_unresolved, repeated_missing,
        repeated_invalid_targets);

    std::printf(
        "fixed split raw endpoint sizes=%zu/%zu bounded0=%zu/%zu\n",
        class_ii_neighbor2_fixed_rank1_endpoint(7).size(),
        class_ii_neighbor2_fixed_rank1_endpoint(8).size(),
        class_ii_neighbor2_fixed_bounded_targets_after(7, 0).size(),
        class_ii_neighbor2_fixed_bounded_targets_after(8, 0).size());
    std::fflush(stdout);
    const auto fixed_rank1_endpoint =
        class_ii_neighbor2_fixed_rank1_endpoint_forms();
    const auto fixed_rank1_bounded =
        class_ii_neighbor2_fixed_bounded_target_forms_after(0);
    const auto evaluate_forms = [](const auto& forms, long long a) {
        std::set<SNode<3>> nodes;
        for (const auto& form : forms)
            nodes.insert(class_ii_evaluate_affine_node_form(form, a));
        return nodes;
    };
    std::printf(
        "fixed split continuation@20 endpoint=%d bounded=%d\n",
        evaluate_forms(fixed_rank1_endpoint, 20)
                == class_ii_neighbor2_fixed_rank1_endpoint(20),
        evaluate_forms(fixed_rank1_bounded, 20)
                == class_ii_neighbor2_fixed_bounded_targets_after(20, 0));
    std::size_t fixed_source_rank1_overlap = 0;
    const auto fixed_ranks_at_eight =
        class_ii_neighbor2_fixed_pruning_ranks(8);
    for (const auto& node : fixed_sources)
        fixed_source_rank1_overlap +=
            fixed_ranks_at_eight[0].count(node);
    const std::vector<ClassIIPruningConeForm> fixed_cell_bounded_domain = {
        {-8, 1, 0, 0},
        {-5, 0, 1, 0}, {-4, 1, -1, 0},
        {0, -1, 0, 1}, {0, 1, 0, -1},
    };
    const std::vector<ClassIIPruningConeForm> fixed_cell_shell_domain = {
        {-8, 1, 0, 0},
        {-5, 0, 1, 0}, {-4, 1, -1, 0},
        {-4, 0, 0, 1}, {-2, 1, 0, -1},
    };
    const std::vector<ClassIIPruningConeForm> fixed_endpoint_shell_domain = {
        {-8, 1, 0, 0},
        {0, -1, 1, 0}, {0, 1, -1, 0},
        {-4, 0, 0, 1}, {-2, 1, 0, -1},
    };
    const auto fixed_rank1_fixed =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_constant_node_forms(fixed_sources),
            fixed_rank1_bounded, 0, 0, 8);
    const auto fixed_rank1_fixed_shells =
        class_ii_neighbor2_affine_pair_exclusion(
            class_ii_constant_node_forms(fixed_sources),
            class_ii_center_interior_shell_forms(),
            false, fixed_endpoint_shell_domain);
    const auto fixed_rank1_cells_bounded =
        class_ii_neighbor2_affine_pair_exclusion(
            class_ii_neighbor2_pruning_cell_forms(),
            fixed_rank1_bounded, false, fixed_cell_bounded_domain);
    const auto fixed_rank1_cells_shells =
        class_ii_neighbor2_affine_pair_exclusion(
            class_ii_neighbor2_pruning_cell_forms(),
            class_ii_center_interior_shell_forms(),
            false, fixed_cell_shell_domain);
    const auto fixed_rank1_endpoint_bounded =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            fixed_rank1_endpoint, fixed_rank1_bounded, 0, 0, 8);
    const auto fixed_rank1_endpoint_shells =
        class_ii_neighbor2_affine_pair_exclusion(
            fixed_rank1_endpoint,
            class_ii_center_interior_shell_forms(),
            false, fixed_endpoint_shell_domain);
    const std::vector<ClassIIAffinePairExclusionResult> rank1_parts = {
        fixed_rank1_fixed, fixed_rank1_fixed_shells,
        fixed_rank1_cells_bounded, fixed_rank1_cells_shells,
        fixed_rank1_endpoint_bounded, fixed_rank1_endpoint_shells,
    };
    std::size_t fixed_rank1_branches = 0;
    std::size_t fixed_rank1_unresolved = 0;
    bool fixed_rank1_exact = true;
    for (const auto& part : rank1_parts) {
        fixed_rank1_branches += part.prefix_branches;
        fixed_rank1_unresolved += part.unresolved_branches;
        fixed_rank1_exact = fixed_rank1_exact && part.exact;
    }
    std::printf(
        "fixed rank1 part unresolved=%zu/%zu/%zu/%zu/%zu/%zu\n",
        fixed_rank1_fixed.unresolved_branches,
        fixed_rank1_fixed_shells.unresolved_branches,
        fixed_rank1_cells_bounded.unresolved_branches,
        fixed_rank1_cells_shells.unresolved_branches,
        fixed_rank1_endpoint_bounded.unresolved_branches,
        fixed_rank1_endpoint_shells.unresolved_branches);
    for (const auto& [source, target] :
         fixed_rank1_cells_bounded.unresolved_pairs)
        std::printf(
            "  unresolved fixed cell pair "
            "src=(%lld,[%lld%+lldq,%lld%+lldq,%lld%+lldq],%lld) "
            "dst=(%lld,[%lld%+llda,%lld%+llda,%lld%+llda],%lld)\n",
            source.i,
            source.x[0].intercept, source.x[0].slope,
            source.x[1].intercept, source.x[1].slope,
            source.x[2].intercept, source.x[2].slope, source.j,
            target.i,
            target.x[0].intercept, target.x[0].slope,
            target.x[1].intercept, target.x[1].slope,
            target.x[2].intercept, target.x[2].slope, target.j);
    std::size_t fixed_rank1_nonzero_at_eight = 0;
    std::size_t fixed_rank1_nonzero_sweep = 0;
    for (const auto* part :
         {&fixed_rank1_fixed, &fixed_rank1_cells_bounded,
          &fixed_rank1_endpoint_bounded}) {
        for (const auto& [source, target] : part->unresolved_pairs) {
            fixed_rank1_nonzero_at_eight +=
                class_ii_neighbor_transition_weight(
                    2, 8,
                    class_ii_evaluate_affine_node_form(source, 8),
                    class_ii_evaluate_affine_node_form(target, 8)) != 0;
            for (long long a = 7; a <= 20; ++a)
                fixed_rank1_nonzero_sweep +=
                    class_ii_neighbor_transition_weight(
                        2, a,
                        class_ii_evaluate_affine_node_form(source, a),
                        class_ii_evaluate_affine_node_form(target, a)) != 0;
        }
    }
    std::printf(
        "fixed rank1 sources=180+48+%zu bounded=%zu branches=%zu "
        "fixed-overlap=%zu unresolved=%zu nonzero@8/sweep=%zu/%zu "
        "exact=%d\n",
        fixed_rank1_endpoint.size(), fixed_rank1_bounded.size(),
        fixed_rank1_branches, fixed_source_rank1_overlap,
        fixed_rank1_unresolved,
        fixed_rank1_nonzero_at_eight,
        fixed_rank1_nonzero_sweep,
        fixed_rank1_exact ? 1 : 0);
    for (std::size_t rank = 1; rank < 6; ++rank) {
        const auto raw7 = class_ii_neighbor2_fixed_pruning_ranks(7);
        const auto raw8 = class_ii_neighbor2_fixed_pruning_ranks(8);
        const auto raw9 = class_ii_neighbor2_fixed_pruning_ranks(9);
        std::printf(
            "fixed rank%zu raw=%zu/%zu/%zu bounded=%zu/%zu/%zu\n",
            rank + 1, raw7[rank].size(), raw8[rank].size(),
            raw9[rank].size(),
            class_ii_neighbor2_fixed_bounded_targets_after(7, rank).size(),
            class_ii_neighbor2_fixed_bounded_targets_after(8, rank).size(),
            class_ii_neighbor2_fixed_bounded_targets_after(9, rank).size());
        std::fflush(stdout);
        const auto sources_at_rank =
            class_ii_neighbor2_fixed_rank_forms(rank);
        const auto bounded_targets =
            class_ii_neighbor2_fixed_bounded_target_forms_after(rank);
        const auto raw20 = class_ii_neighbor2_fixed_pruning_ranks(20);
        std::printf(
            "fixed rank%zu continuation@20 source=%d bounded=%d\n",
            rank + 1,
            evaluate_forms(sources_at_rank, 20) == raw20[rank],
            evaluate_forms(bounded_targets, 20)
                == class_ii_neighbor2_fixed_bounded_targets_after(
                    20, rank));
        const auto bounded =
            class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
                sources_at_rank, bounded_targets, 0, 0, 8);
        const auto shells =
            class_ii_neighbor2_affine_pair_exclusion(
                sources_at_rank,
                class_ii_center_interior_shell_forms(),
                false, fixed_endpoint_shell_domain);
        std::printf(
            "fixed rank%zu sources=%zu bounded=%zu branches=%zu "
            "unresolved=%zu exact=%d\n",
            rank + 1, sources_at_rank.size(), bounded_targets.size(),
            bounded.prefix_branches + shells.prefix_branches,
            bounded.unresolved_branches + shells.unresolved_branches,
            bounded.exact && shells.exact ? 1 : 0);
    }

    {
        constexpr long long a = 7;
        const auto nodes =
            class_ii_neighbor2_fixed_pre_red_decomposed(a);
        std::vector<std::tuple<SNode<3>, SNode<3>,
                               std::vector<long long>,
                               std::vector<long long>>> edges;
        for (const auto& source : nodes)
            for (const auto& target : nodes)
                if (class_ii_neighbor_transition_weight(
                        2, a, source, target) != 0)
                    edges.emplace_back(
                        source, target,
                        std::vector<long long>{},
                        std::vector<long long>{});
        const auto actual = red_trace<3>(nodes, edges);
        const auto claimed =
            class_ii_neighbor2_fixed_pruning_ranks(a);
        bool exact = actual.pruning_ranks == claimed;
        std::printf(
            "fixed Red base a=7 nodes=%zu edges=%zu ranks=%zu exact=%d\n",
            nodes.size(), edges.size(), actual.pruning_ranks.size(),
            exact ? 1 : 0);
    }
    std::size_t fixed_predecessors = 0;
    bool fixed_predecessors_exact = true;
    for (std::size_t rank = 1; rank < 6; ++rank) {
        const auto sources_at_rank =
            class_ii_neighbor2_fixed_rank_forms(rank);
        const auto ranks8 = class_ii_neighbor2_fixed_pruning_ranks(8);
        const auto ranks9 = class_ii_neighbor2_fixed_pruning_ranks(9);
        std::size_t found_count = 0;
        for (const auto& source_form : sources_at_rank) {
            const auto source8 =
                class_ii_evaluate_affine_node_form(source_form, 8);
            const auto source9 =
                class_ii_evaluate_affine_node_form(source_form, 9);
            bool found = false;
            for (const auto& target8 : ranks8[rank - 1]) {
                if (class_ii_neighbor_transition_weight(
                        2, 8, source8, target8) <= 0)
                    continue;
                for (const auto& target9 : ranks9[rank - 1]) {
                    if (target8.i != target9.i
                            || target8.j != target9.j
                            || target8.x[2] != target9.x[2]
                            || target8.x[0] + target8.x[1]
                                != target9.x[0] + target9.x[1]
                            || std::abs(target9.x[0] - target8.x[0]) > 1
                            || std::abs(target9.x[1] - target8.x[1]) > 1
                            || class_ii_neighbor_transition_weight(
                                2, 9, source9, target9) <= 0)
                        continue;
                    ClassIINodeAffineParameterForm target_form;
                    target_form.i = target8.i;
                    target_form.j = target8.j;
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        const long long slope =
                            target9.x[coordinate]
                            - target8.x[coordinate];
                        target_form.x[coordinate] = {
                            target8.x[coordinate] - 8 * slope, slope};
                    }
                    const ClassIIAffineRedEdge edge{
                        source_form, target_form};
                    if (class_ii_neighbor2_affine_edge_weight_positive(
                            edge, 8, 0, 0)) {
                        found = true;
                        ++found_count;
                        break;
                    }
                }
                if (found) break;
            }
        }
        fixed_predecessors += found_count;
        fixed_predecessors_exact =
            fixed_predecessors_exact
            && found_count == sources_at_rank.size();
        std::printf(
            "fixed rank%zu predecessors=%zu/%zu\n",
            rank + 1, found_count, sources_at_rank.size());
    }
    std::printf(
        "fixed later-rank predecessors total=%zu/97 exact=%d\n",
        fixed_predecessors, fixed_predecessors_exact ? 1 : 0);

    return fixed.exact && cells.exact && endpoint.exact ? 0 : 1;
}
