#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace ravel::proof {

// Small, syntax-only emitters shared by renderer components.  They do not
// inspect provenance or make proof decisions; they only serialize concrete
// values into Lean literals.
inline std::string render_lean_int_matrix(const std::vector<long long>& flat, long long n) {
    std::ostringstream out;
    out << "!![";
    for (long long i = 0; i < n; ++i) {
        if (i > 0) out << "; ";
        for (long long j = 0; j < n; ++j) {
            if (j > 0) out << ", ";
            out << flat[static_cast<std::size_t>(i * n + j)];
        }
    }
    out << "]";
    return out.str();
}

inline std::string render_lean_int_vector(const std::vector<long long>& values) {
    std::ostringstream out;
    out << "(![";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out << ", ";
        out << values[i];
    }
    out << "] : Fin " << values.size() << " → ℤ)";
    return out.str();
}

inline std::string render_lean_fin_list(const std::vector<long long>& letters) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < letters.size(); ++i) {
        if (i > 0) out << ", ";
        out << letters[i];
    }
    out << "]";
    return out.str();
}

inline std::string render_lean_nat_list_of_list(
    const std::vector<std::vector<long long>>& images) {
    std::ostringstream out;
    out << '[';
    for (std::size_t i = 0; i < images.size(); ++i) {
        if (i) out << ", ";
        out << '[';
        for (std::size_t j = 0; j < images[i].size(); ++j) {
            if (j) out << ", ";
            out << images[i][j];
        }
        out << ']';
    }
    out << ']';
    return out.str();
}

inline std::string render_nat_finset(const std::vector<long long>& values) {
    std::ostringstream out;
    out << "{";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out << ", ";
        out << values[i];
    }
    out << "}";
    return out.str();
}

} // namespace ravel::proof
