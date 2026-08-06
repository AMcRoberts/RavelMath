#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

/** A quotient-edge channel.  Distinct channels may have the same source and
 * target quotient classes; they are distinct base multiedges. */
struct FiberedQuotientChannel {
    std::size_t id = 0;
    std::size_t source_class = 0;
    std::size_t target_class = 0;
};

/** A concrete lifted edge tagged by the quotient channel it lies above. */
struct FiberedConcreteEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t channel = 0;
};

struct FiberedTwistedQuotientCertificate {
    std::size_t concrete_states = 0;
    std::size_t quotient_classes = 0;
    std::size_t channels = 0;
    std::size_t concrete_edges = 0;
    std::size_t connected_class_components = 0;
    std::size_t nontrivial_holonomy_generators = 0;
    std::vector<std::size_t> class_sizes;
    std::vector<std::vector<std::size_t>> class_members;
    // For each channel, the target local index for each source local index.
    std::vector<std::vector<std::size_t>> channel_bijections;
    std::vector<std::vector<std::size_t>> quotient_adjacency;
    std::vector<std::vector<std::size_t>> concrete_adjacency;
    bool classes_nonempty = false;
    bool channel_endpoints_well_defined = false;
    bool channel_lifts_total = false;
    bool channel_lifts_single_valued = false;
    bool channel_lifts_bijective = false;
    bool local_fibre_sizes_compatible = false;
    bool unique_path_lifting = false;
    bool path_counts_preserved = false;
    bool spectral_radius_preserved = false;
    bool genuinely_twisted_inside_classes = false;
    bool valid = false;
    std::string obstruction;
};

namespace fibered_twisted_detail {

inline std::vector<std::vector<std::size_t>> members_by_class(
    const std::vector<std::size_t>& state_class,
    std::size_t class_count) {
    std::vector<std::vector<std::size_t>> members(class_count);
    for (std::size_t s = 0; s < state_class.size(); ++s) {
        if (state_class[s] >= class_count)
            throw std::invalid_argument("fibered quotient: bad state class");
        members[state_class[s]].push_back(s);
    }
    return members;
}

inline std::vector<std::size_t> local_indices(
    const std::vector<std::vector<std::size_t>>& members,
    std::size_t state_count) {
    std::vector<std::size_t> local(state_count, 0);
    for (const auto& fibre : members)
        for (std::size_t i = 0; i < fibre.size(); ++i)
            local[fibre[i]] = i;
    return local;
}

inline std::vector<std::size_t> compose_permutations(
    const std::vector<std::size_t>& a,
    const std::vector<std::size_t>& b) {
    if (a.size() != b.size()) return {};
    std::vector<std::size_t> c(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] >= b.size()) return {};
        c[i] = b[a[i]];
    }
    return c;
}

inline bool identity_permutation(const std::vector<std::size_t>& p) {
    for (std::size_t i = 0; i < p.size(); ++i)
        if (p[i] != i) return false;
    return true;
}

} // namespace fibered_twisted_detail

/** Derive a finite quotient with twisted subelements.
 *
 * There is deliberately no global fibre coordinate.  Each quotient class has
 * its own finite set of concrete representatives.  A quotient channel is
 * accepted exactly when its concrete lift is a bijection from the source
 * class representatives to the target class representatives.  Therefore a
 * quotient path has one and only one concrete lift from each chosen starting
 * representative.  Channel bijections can have nontrivial holonomy around a
 * quotient cycle: this is the internal twist of quotient elements.
 *
 * The theorem is the finite-covering analogue of a permutation skew product,
 * but it does not assume that the cover globally trivializes as B x F.
 */
inline FiberedTwistedQuotientCertificate derive_fibered_twisted_quotient(
    const std::vector<std::size_t>& state_class,
    std::size_t class_count,
    const std::vector<FiberedQuotientChannel>& channels,
    const std::vector<FiberedConcreteEdge>& concrete_edges,
    std::size_t replay_depth = 12) {
    FiberedTwistedQuotientCertificate c;
    c.concrete_states = state_class.size();
    c.quotient_classes = class_count;
    c.channels = channels.size();
    c.concrete_edges = concrete_edges.size();
    if (state_class.empty() || class_count == 0) {
        c.obstruction = "fibered quotient requires states and quotient classes";
        return c;
    }

    c.class_members = fibered_twisted_detail::members_by_class(state_class, class_count);
    c.class_sizes.resize(class_count);
    c.classes_nonempty = true;
    for (std::size_t q = 0; q < class_count; ++q) {
        c.class_sizes[q] = c.class_members[q].size();
        c.classes_nonempty &= !c.class_members[q].empty();
    }
    if (!c.classes_nonempty) {
        c.obstruction = "fibered quotient has an empty quotient class";
        return c;
    }

    std::map<std::size_t, std::size_t> channel_index;
    c.channel_endpoints_well_defined = true;
    for (std::size_t k = 0; k < channels.size(); ++k) {
        const auto& e = channels[k];
        c.channel_endpoints_well_defined &= e.source_class < class_count &&
                                            e.target_class < class_count;
        c.channel_endpoints_well_defined &= channel_index.emplace(e.id, k).second;
    }
    if (!c.channel_endpoints_well_defined) {
        c.obstruction = "fibered quotient has malformed or duplicate channels";
        return c;
    }

    const auto local = fibered_twisted_detail::local_indices(c.class_members, state_class.size());
    c.channel_bijections.resize(channels.size());
    std::vector<std::vector<int>> forward(channels.size());
    std::vector<std::vector<int>> inverse(channels.size());
    for (std::size_t k = 0; k < channels.size(); ++k) {
        const auto& e = channels[k];
        c.local_fibre_sizes_compatible |=
            c.class_sizes[e.source_class] == c.class_sizes[e.target_class];
        forward[k].assign(c.class_sizes[e.source_class], -1);
        inverse[k].assign(c.class_sizes[e.target_class], -1);
    }
    // The |= above records at least one compatible edge; reset and require all.
    c.local_fibre_sizes_compatible = true;
    for (const auto& e : channels)
        c.local_fibre_sizes_compatible &=
            c.class_sizes[e.source_class] == c.class_sizes[e.target_class];
    if (!c.local_fibre_sizes_compatible) {
        c.obstruction = "a quotient channel joins classes of unequal local fibre size";
        return c;
    }

    c.concrete_adjacency.assign(state_class.size(), {});
    c.quotient_adjacency.assign(class_count, {});
    for (const auto& e : channels)
        c.quotient_adjacency[e.source_class].push_back(e.target_class);

    c.channel_lifts_single_valued = true;
    c.channel_lifts_bijective = true;
    for (const auto& e : concrete_edges) {
        const auto it = channel_index.find(e.channel);
        if (it == channel_index.end() || e.source >= state_class.size() ||
            e.target >= state_class.size()) {
            c.obstruction = "concrete edge references a missing channel or state";
            return c;
        }
        const auto k = it->second;
        const auto& ch = channels[k];
        if (state_class[e.source] != ch.source_class ||
            state_class[e.target] != ch.target_class) {
            c.obstruction = "concrete edge does not lie over its announced quotient channel";
            return c;
        }
        const auto a = local[e.source];
        const auto b = local[e.target];
        if (forward[k][a] >= 0 && forward[k][a] != static_cast<int>(b))
            c.channel_lifts_single_valued = false;
        if (inverse[k][b] >= 0 && inverse[k][b] != static_cast<int>(a))
            c.channel_lifts_bijective = false;
        forward[k][a] = static_cast<int>(b);
        inverse[k][b] = static_cast<int>(a);
        c.concrete_adjacency[e.source].push_back(e.target);
    }
    if (!c.channel_lifts_single_valued || !c.channel_lifts_bijective) {
        c.obstruction = "a quotient channel is not represented by a partial bijection";
        return c;
    }

    c.channel_lifts_total = true;
    for (std::size_t k = 0; k < channels.size(); ++k) {
        for (const auto b : forward[k]) c.channel_lifts_total &= b >= 0;
        for (const auto a : inverse[k]) c.channel_lifts_bijective &= a >= 0;
        c.channel_bijections[k].resize(forward[k].size());
        for (std::size_t i = 0; i < forward[k].size(); ++i)
            if (forward[k][i] >= 0)
                c.channel_bijections[k][i] = static_cast<std::size_t>(forward[k][i]);
    }
    if (!c.channel_lifts_total || !c.channel_lifts_bijective) {
        c.obstruction = "a quotient channel does not lift bijectively on every subelement";
        return c;
    }

    c.unique_path_lifting = true;

    // Detect nontrivial internal twist without choosing a global fibre gauge.
    // Transport local indices along a spanning forest of the quotient graph;
    // every non-tree channel then gives a holonomy permutation in its source
    // class coordinates.  A nonidentity permutation is a genuine twist among
    // quotient subelements, not a twist assigned to the whole quotient.
    std::vector<int> seen(class_count, 0);
    std::vector<std::vector<std::size_t>> chart(class_count);
    std::vector<std::vector<std::pair<std::size_t,std::size_t>>> undirected(class_count);
    for (std::size_t k = 0; k < channels.size(); ++k) {
        undirected[channels[k].source_class].push_back({channels[k].target_class, k});
        undirected[channels[k].target_class].push_back({channels[k].source_class, k});
    }
    for (std::size_t root = 0; root < class_count; ++root) {
        if (seen[root]) continue;
        ++c.connected_class_components;
        chart[root].resize(c.class_sizes[root]);
        std::iota(chart[root].begin(), chart[root].end(), 0);
        seen[root] = 1;
        std::queue<std::size_t> q;
        q.push(root);
        while (!q.empty()) {
            const auto u = q.front(); q.pop();
            for (const auto [v, k] : undirected[u]) {
                const auto& ch = channels[k];
                std::vector<std::size_t> transport = c.channel_bijections[k];
                if (u == ch.target_class) {
                    std::vector<std::size_t> inv(transport.size());
                    for (std::size_t i = 0; i < transport.size(); ++i)
                        inv[transport[i]] = i;
                    transport = std::move(inv);
                }
                if (!seen[v]) {
                    chart[v].resize(transport.size());
                    for (std::size_t i = 0; i < transport.size(); ++i)
                        chart[v][transport[i]] = chart[u][i];
                    seen[v] = 1;
                    q.push(v);
                } else {
                    // h maps the root chart to itself around this closed walk.
                    std::vector<std::size_t> h(transport.size());
                    std::vector<std::size_t> inverse_chart_v(chart[v].size());
                    for (std::size_t i = 0; i < chart[v].size(); ++i)
                        inverse_chart_v[chart[v][i]] = i;
                    for (std::size_t i = 0; i < transport.size(); ++i)
                        h[chart[u][i]] = chart[v][transport[i]];
                    if (!fibered_twisted_detail::identity_permutation(h))
                        ++c.nontrivial_holonomy_generators;
                }
            }
        }
    }
    c.genuinely_twisted_inside_classes = c.nontrivial_holonomy_generators > 0;

    // Replay path counts.  Each concrete starting subelement above a quotient
    // class must have exactly the quotient path count at every depth.
    std::vector<std::uint64_t> base_counts(class_count, 1);
    std::vector<std::uint64_t> concrete_counts(state_class.size(), 1);
    c.path_counts_preserved = true;
    for (std::size_t depth = 0; depth <= replay_depth; ++depth) {
        for (std::size_t s = 0; s < state_class.size(); ++s)
            c.path_counts_preserved &= concrete_counts[s] == base_counts[state_class[s]];
        std::vector<std::uint64_t> next_base(class_count, 0);
        std::vector<std::uint64_t> next_concrete(state_class.size(), 0);
        for (std::size_t u = 0; u < class_count; ++u)
            for (const auto v : c.quotient_adjacency[u])
                next_base[u] += base_counts[v];
        for (std::size_t u = 0; u < state_class.size(); ++u)
            for (const auto v : c.concrete_adjacency[u])
                next_concrete[u] += concrete_counts[v];
        base_counts = std::move(next_base);
        concrete_counts = std::move(next_concrete);
    }
    c.spectral_radius_preserved = c.unique_path_lifting && c.path_counts_preserved;
    c.valid = c.classes_nonempty && c.channel_endpoints_well_defined &&
              c.local_fibre_sizes_compatible && c.channel_lifts_total &&
              c.channel_lifts_single_valued && c.channel_lifts_bijective &&
              c.unique_path_lifting && c.path_counts_preserved &&
              c.spectral_radius_preserved;
    if (!c.valid && c.obstruction.empty())
        c.obstruction = "fibered twisted quotient certificate failed";
    return c;
}

} // namespace ravel::proof
