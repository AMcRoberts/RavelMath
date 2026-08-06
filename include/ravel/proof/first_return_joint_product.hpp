#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/symbolic_residual_induction.hpp"

namespace ravel::proof {

struct FirstReturnWitness {
    std::size_t dimension = 0;
    std::int64_t bound = 0;
    std::vector<std::int64_t> digits;
    std::vector<std::vector<std::int64_t>> base_path;
    std::vector<std::int64_t> target;
};

struct JointFirstReturnState {
    std::vector<std::int64_t> plant_state;
    std::size_t residual = 0;
    std::size_t remaining = 0;
    std::vector<std::pair<std::size_t, std::int64_t>> target_faces;

    friend bool operator==(const JointFirstReturnState&, const JointFirstReturnState&) = default;

    friend bool operator<(const JointFirstReturnState& a,
                          const JointFirstReturnState& b) {
        return std::tie(a.plant_state, a.residual, a.remaining, a.target_faces) <
               std::tie(b.plant_state, b.residual, b.remaining, b.target_faces);
    }
};

struct JointFirstReturnTransition {
    JointFirstReturnState source;
    std::int64_t digit = 0;
    JointFirstReturnState target;

    friend bool operator<(const JointFirstReturnTransition& a,
                          const JointFirstReturnTransition& b) {
        return std::tie(a.source, a.digit, a.target) <
               std::tie(b.source, b.digit, b.target);
    }
};

struct FirstReturnJointProduct {
    std::size_t dimension = 0;
    std::size_t witness_count = 0;
    std::set<JointFirstReturnState> reachable;
    std::set<JointFirstReturnTransition> transitions;
    std::map<std::tuple<std::size_t, std::int64_t>, std::size_t>
        residual_predecessor;
    std::vector<SymbolicControllerState> controller_states;
    ControllerPlant controller_plant;
    std::vector<ResidualController> residual_members;
    std::map<std::size_t, std::vector<std::pair<std::size_t, std::int64_t>>>
        terminal_faces_by_residual;
    bool replayed = false;
    std::string failure;
};

namespace detail {

inline std::string read_text(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path.string());
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

inline void skip_ws(const std::string& s, std::size_t& i) {
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
}

inline std::int64_t parse_int(const std::string& s, std::size_t& i) {
    skip_ws(s, i);
    bool neg = false;
    if (i < s.size() && s[i] == '-') { neg = true; ++i; }
    if (i >= s.size() || !std::isdigit(static_cast<unsigned char>(s[i])))
        throw std::runtime_error("expected integer");
    std::int64_t v = 0;
    while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
        v = 10 * v + (s[i] - '0');
        ++i;
    }
    return neg ? -v : v;
}

inline std::vector<std::int64_t> parse_int_array_at(const std::string& s,
                                                     std::size_t& i) {
    skip_ws(s, i);
    if (i >= s.size() || s[i] != '[') throw std::runtime_error("expected array");
    ++i;
    std::vector<std::int64_t> out;
    for (;;) {
        skip_ws(s, i);
        if (i < s.size() && s[i] == ']') { ++i; break; }
        out.push_back(parse_int(s, i));
        skip_ws(s, i);
        if (i < s.size() && s[i] == ',') { ++i; continue; }
        if (i < s.size() && s[i] == ']') { ++i; break; }
        throw std::runtime_error("malformed integer array");
    }
    return out;
}

inline std::vector<std::vector<std::int64_t>> parse_matrix_at(
    const std::string& s, std::size_t& i) {
    skip_ws(s, i);
    if (i >= s.size() || s[i] != '[') throw std::runtime_error("expected matrix");
    ++i;
    std::vector<std::vector<std::int64_t>> out;
    for (;;) {
        skip_ws(s, i);
        if (i < s.size() && s[i] == ']') { ++i; break; }
        out.push_back(parse_int_array_at(s, i));
        skip_ws(s, i);
        if (i < s.size() && s[i] == ',') { ++i; continue; }
        if (i < s.size() && s[i] == ']') { ++i; break; }
        throw std::runtime_error("malformed matrix");
    }
    return out;
}

inline std::size_t find_value(const std::string& object,
                              const std::string& key) {
    const auto marker = std::string("\"") + key + "\"";
    auto p = object.find(marker);
    if (p == std::string::npos) throw std::runtime_error("missing key " + key);
    p = object.find(':', p + marker.size());
    if (p == std::string::npos) throw std::runtime_error("missing colon");
    return p + 1;
}

inline std::vector<std::string> split_object_array(const std::string& s,
                                                   const std::string& key) {
    auto i = find_value(s, key);
    skip_ws(s, i);
    if (i >= s.size() || s[i] != '[') throw std::runtime_error("expected object array");
    ++i;
    std::vector<std::string> objects;
    for (;;) {
        skip_ws(s, i);
        if (i < s.size() && s[i] == ']') break;
        if (i >= s.size() || s[i] != '{') throw std::runtime_error("expected object");
        const auto begin = i;
        int depth = 0;
        bool in_string = false;
        bool escape = false;
        for (; i < s.size(); ++i) {
            const char c = s[i];
            if (in_string) {
                if (escape) escape = false;
                else if (c == '\\') escape = true;
                else if (c == '"') in_string = false;
                continue;
            }
            if (c == '"') { in_string = true; continue; }
            if (c == '{') ++depth;
            else if (c == '}' && --depth == 0) { ++i; break; }
        }
        objects.push_back(s.substr(begin, i - begin));
        skip_ws(s, i);
        if (i < s.size() && s[i] == ',') ++i;
    }
    return objects;
}

inline std::vector<std::pair<std::size_t, std::int64_t>> target_faces(
    const std::vector<std::int64_t>& target, std::int64_t bound) {
    std::vector<std::pair<std::size_t, std::int64_t>> out;
    for (std::size_t i = 0; i < target.size(); ++i) {
        if (std::llabs(target[i]) == bound)
            out.emplace_back(i, target[i] > 0 ? 1 : -1);
    }
    return out;
}

inline ResidualController face_monitor(
    const ResidualInductionProof& proof,
    const std::vector<std::pair<std::size_t, std::int64_t>>& faces) {
    ResidualController monitor(proof.states.size(), false);
    for (std::size_t s = 0; s < proof.states.size(); ++s) {
        for (const auto& [coord, sign] : faces) {
            if (coord < proof.states[s].size() && proof.states[s][coord] == sign) {
                monitor[s] = true;
                break;
            }
        }
    }
    return monitor;
}

} // namespace detail

inline std::vector<FirstReturnWitness> load_first_return_witnesses(
    const std::filesystem::path& path) {
    const auto text = detail::read_text(path);
    auto p = detail::find_value(text, "bound");
    const auto bound = detail::parse_int(text, p);
    const auto objects = detail::split_object_array(text, "certificates");
    std::vector<FirstReturnWitness> result;
    result.reserve(objects.size());
    for (const auto& object : objects) {
        FirstReturnWitness w;
        w.bound = bound;
        auto q = detail::find_value(object, "digits");
        w.digits = detail::parse_int_array_at(object, q);
        q = detail::find_value(object, "base_path");
        w.base_path = detail::parse_matrix_at(object, q);
        q = detail::find_value(object, "target");
        w.target = detail::parse_int_array_at(object, q);
        w.dimension = w.target.size();
        if (w.base_path.size() != w.digits.size() + 1)
            throw std::runtime_error("first-return witness path length mismatch");
        for (const auto& state : w.base_path)
            if (state.size() != w.dimension)
                throw std::runtime_error("first-return witness dimension mismatch");
        result.push_back(std::move(w));
    }
    return result;
}

inline FirstReturnJointProduct build_first_return_joint_product(
    std::size_t dimension,
    const std::vector<FirstReturnWitness>& witnesses) {
    FirstReturnJointProduct product;
    product.dimension = dimension;
    const auto states = enumerate_radius_one_states(dimension);
    const auto plant = make_symbolic_controller_plant(states);
    product.controller_states = states;
    product.controller_plant = plant;

    std::vector<ResidualController> generators;
    std::map<std::vector<std::pair<std::size_t, std::int64_t>>, ResidualController>
        monitor_by_faces;
    for (const auto& witness : witnesses) {
        if (witness.dimension != dimension) continue;
        const auto faces = detail::target_faces(witness.target, witness.bound);
        if (monitor_by_faces.contains(faces)) continue;
        ResidualController monitor(states.size(), false);
        for (std::size_t s = 0; s < states.size(); ++s) {
            for (const auto& [coord, sign] : faces) {
                if (coord < states[s].size() && states[s][coord] == sign) {
                    monitor[s] = true;
                    break;
                }
            }
        }
        monitor_by_faces.emplace(faces, monitor);
        generators.push_back(std::move(monitor));
    }
    const auto family = close_controller_family(plant, generators, {-1, 0, 1});
    if (!family.closed) {
        product.failure = "witness-conditioned residual family did not close";
        return product;
    }
    product.residual_members = family.members;
    for (const auto& [faces, monitor] : monitor_by_faces) {
        const auto it = family.member_ids.find(monitor);
        if (it == family.member_ids.end()) {
            product.failure = "terminal monitor missing from closed family";
            return product;
        }
        product.terminal_faces_by_residual[it->second] = faces;
    }
    for (const auto& [key, target] : family.predecessor_table)
        product.residual_predecessor.emplace(key, target);

    for (const auto& witness : witnesses) {
        if (witness.dimension != dimension) continue;
        ++product.witness_count;
        const auto faces = detail::target_faces(witness.target, witness.bound);
        const auto mit = monitor_by_faces.find(faces);
        if (mit == monitor_by_faces.end()) {
            product.failure = "missing witness endpoint monitor";
            return product;
        }
        const auto fit = family.member_ids.find(mit->second);
        if (fit == family.member_ids.end()) {
            product.failure = "endpoint monitor missing from conditioned residual family";
            return product;
        }
        std::size_t residual = fit->second;
        JointFirstReturnState suffix{witness.base_path.back(), residual, 0, faces};
        product.reachable.insert(suffix);
        for (std::size_t k = witness.digits.size(); k-- > 0;) {
            const auto digit = witness.digits[k];
            const auto key = std::make_tuple(residual, digit);
            const auto pit = family.predecessor_table.find(key);
            if (pit == family.predecessor_table.end()) {
                product.failure = "missing exact predecessor transition";
                return product;
            }
            residual = pit->second;
            JointFirstReturnState prefix{witness.base_path[k], residual,
                                         witness.digits.size() - k, faces};
            product.reachable.insert(prefix);
            product.transitions.insert({prefix, digit, suffix});
            suffix = std::move(prefix);
        }
    }
    product.replayed = product.failure.empty();
    return product;
}

inline FirstReturnJointProduct build_first_return_joint_product_from_files(
    std::size_t dimension,
    const std::vector<std::filesystem::path>& files) {
    std::vector<FirstReturnWitness> witnesses;
    for (const auto& file : files) {
        auto part = load_first_return_witnesses(file);
        witnesses.insert(witnesses.end(),
                         std::make_move_iterator(part.begin()),
                         std::make_move_iterator(part.end()));
    }
    return build_first_return_joint_product(dimension, witnesses);
}

} // namespace ravel::proof
