#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

struct PhaseRankEdge {
    std::string source;
    std::string target;
    std::int64_t required_gain = 0;
};

struct PhaseRankTransportCertificate {
    std::string certificate_id;
    std::vector<std::string> phases;
    std::vector<PhaseRankEdge> constraints;
    std::map<std::string, std::int64_t> offsets;
    std::vector<std::string> covered_instances;
    bool feasible = false;
    bool replay_checked = false;
};

inline std::optional<std::map<std::string, std::int64_t>>
solve_phase_rank_offsets(const std::vector<std::string>& phases,
                         const std::vector<PhaseRankEdge>& edges) {
    std::map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < phases.size(); ++i) {
        if (!index.emplace(phases[i], i).second)
            throw std::invalid_argument("duplicate phase");
    }

    std::vector<std::int64_t> height(phases.size(), 0);
    for (std::size_t round = 0; round < phases.size(); ++round) {
        bool changed = false;
        for (const auto& edge : edges) {
            const auto su = index.find(edge.source);
            const auto tv = index.find(edge.target);
            if (su == index.end() || tv == index.end())
                throw std::invalid_argument("unknown phase in constraint");
            const auto candidate = height[su->second] + edge.required_gain;
            if (height[tv->second] < candidate) {
                height[tv->second] = candidate;
                changed = true;
            }
        }
        if (!changed) {
            std::map<std::string, std::int64_t> result;
            for (std::size_t i = 0; i < phases.size(); ++i)
                result.emplace(phases[i], height[i]);
            return result;
        }
    }
    return std::nullopt;
}

inline bool replay_phase_rank_certificate(
    const PhaseRankTransportCertificate& cert) {
    if (!cert.feasible || cert.phases.size() != cert.offsets.size())
        return false;
    for (const auto& phase : cert.phases)
        if (!cert.offsets.contains(phase)) return false;
    for (const auto& edge : cert.constraints) {
        const auto source = cert.offsets.find(edge.source);
        const auto target = cert.offsets.find(edge.target);
        if (source == cert.offsets.end() || target == cert.offsets.end())
            return false;
        if (target->second < source->second + edge.required_gain)
            return false;
    }
    return true;
}

inline PhaseRankTransportCertificate close_phase_rank_transport(
    std::string certificate_id,
    std::vector<std::string> phases,
    std::vector<PhaseRankEdge> constraints,
    std::vector<std::string> covered_instances) {
    std::sort(phases.begin(), phases.end());
    phases.erase(std::unique(phases.begin(), phases.end()), phases.end());

    PhaseRankTransportCertificate cert;
    cert.certificate_id = std::move(certificate_id);
    cert.phases = std::move(phases);
    cert.constraints = std::move(constraints);
    cert.covered_instances = std::move(covered_instances);

    const auto offsets = solve_phase_rank_offsets(cert.phases, cert.constraints);
    cert.feasible = offsets.has_value();
    if (offsets) cert.offsets = *offsets;
    cert.replay_checked = replay_phase_rank_certificate(cert);
    return cert;
}

}  // namespace ravel::proof
