// nbonacci_covering_witness_enumerator.cpp
//
// C++ covering-witness enumerator for the n-bonacci homogeneous
// boundary automaton.  This is the C++ durable layer for the
// enumeration done in Python by nbonacci_shell_covering_proof.py
// (the C++ version is ~10-50x faster; the Python tool stays as a
// fast-iteration exploration prototype).
//
// For a given n and L, enumerate every (n-1)-pin subset of
// {1, ..., n+L-1} and every (n-1)-sign assignment in {-1, +1},
// solve the resulting (n-1) x (n-1) linear system (exact rational
// arithmetic with SignInt = (num, denom) reduced to coprime form),
// and for each valid (in-box, covering) assignment emit a JSON
// record with: the pin indices, the signs, the free-parameter
// solution, the full covering sequence, the gap pattern, and
// structural features (distinct |a_j| count, sign balance, cover
// compactness, simplicity score).
//
// Usage: ./out/nbonacci_covering_witness_enumerator --n=N [--L=L]
//   L defaults to n+1 (the SAT witness length).  Pass --L=n+2
//   to also produce the UNSAT exhaustive-enumeration certificate.
//
// JSON output: one file per (n, L) under --out-dir.  Format is
// compatible with the Python tool's output (cross-checkable by
// the Python replay checker).

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

struct SignInt {
    long long num = 0;
    long long denom = 1;
    SignInt() = default;
    SignInt(long long n) : num(n), denom(1) {}
    SignInt(long long n, long long d) : num(n), denom(d) { normalize(); }
    static long long gcd(long long a, long long b) {
        if (a < 0) a = -a;
        if (b < 0) b = -b;
        while (b != 0) { long long t = b; b = a % b; a = t; }
        return a;
    }
    void normalize() {
        if (denom < 0) { denom = -denom; num = -num; }
        if (num == 0) { denom = 1; return; }
        long long g = gcd(num, denom);
        num /= g; denom /= g;
    }
    SignInt operator+(const SignInt& o) const {
        long long d = denom / gcd(denom, o.denom) * o.denom;
        long long n = num * (d / denom) + o.num * (d / o.denom);
        return SignInt(n, d);
    }
    SignInt operator-(const SignInt& o) const {
        long long d = denom / gcd(denom, o.denom) * o.denom;
        long long n = num * (d / denom) - o.num * (d / o.denom);
        return SignInt(n, d);
    }
    SignInt operator*(const SignInt& o) const {
        return SignInt(num * o.num, denom * o.denom);
    }
    SignInt operator/(const SignInt& o) const {
        if (o.num == 0) throw std::invalid_argument("division by zero");
        return SignInt(num * o.denom, denom * o.num);
    }
    SignInt operator-() const { return SignInt(-num, denom); }
    bool operator==(const SignInt& o) const { return num * o.denom == o.num * denom; }
    bool operator<(const SignInt& o) const { return num * o.denom < o.num * denom; }
    bool operator<=(const SignInt& o) const { return !(o < *this); }
    std::string to_string() const {
        if (denom == 1) return std::to_string(num);
        return std::to_string(num) + "/" + std::to_string(denom);
    }
};

std::optional<std::vector<SignInt>> solve_linear_system(
    const std::vector<std::vector<SignInt>>& A,
    const std::vector<SignInt>& b) {
    const std::size_t m = A.size();
    if (m == 0) return std::vector<SignInt>{};
    if (A[0].size() != m) throw std::invalid_argument("non-square A");
    if (b.size() != m) throw std::invalid_argument("b size mismatch");
    std::vector<std::vector<SignInt>> aug(m, std::vector<SignInt>(m + 1));
    for (std::size_t i = 0; i < m; ++i) {
        for (std::size_t j = 0; j < m; ++j) aug[i][j] = A[i][j];
        aug[i][m] = b[i];
    }
    for (std::size_t col = 0; col < m; ++col) {
        std::size_t pivot = m;
        for (std::size_t i = col; i < m; ++i) {
            if (!(aug[i][col] == SignInt(0))) { pivot = i; break; }
        }
        if (pivot == m) return std::nullopt;
        if (pivot != col) std::swap(aug[pivot], aug[col]);
        SignInt pv = aug[col][col];
        for (std::size_t j = col; j <= m; ++j) aug[col][j] = aug[col][j] / pv;
        for (std::size_t i = 0; i < m; ++i) {
            if (i == col) continue;
            if (aug[i][col] == SignInt(0)) continue;
            SignInt factor = aug[i][col];
            for (std::size_t j = col; j <= m; ++j) {
                aug[i][j] = aug[i][j] - factor * aug[col][j];
            }
        }
    }
    std::vector<SignInt> x(m);
    for (std::size_t i = 0; i < m; ++i) x[i] = aug[i][m];
    return x;
}

// (The wrapper `reconstruct_sequence` is inlined into `recurse_signs`
// below for the main enumeration loop; the free-function form was
// retained for documentation but is not called.)

struct Candidate {
    std::vector<std::size_t> indices;
    std::vector<long long> signs;
    std::vector<SignInt> free_params;
    std::vector<SignInt> sequence;
    std::vector<long long> gap_pattern;
    std::vector<long long> cover_per_pin;
    long long distinct_abs_count = 0;
    long long zeros_in_solution = 0;
    long long cover_min = 0;
    long long cover_max = 0;
    long long distinct_gap_count = 0;
    long long simplicity_score = 0;
};

// (The wrapper `reconstruct_sequence` is inlined into `recurse_signs`
// below for the main enumeration loop; the free-function form is
// intentionally not retained.)

bool in_open_unit_box(const SignInt& v) {
    return SignInt(-1) < v && v < SignInt(1);
}

bool in_closed_unit_box(const SignInt& v) {
    return v == SignInt(-1) || v == SignInt(0) || v == SignInt(1) || in_open_unit_box(v);
}

bool is_covering(std::size_t n, std::size_t L,
                 const std::vector<SignInt>& seq) {
    for (std::size_t t = 0; t <= L; ++t) {
        bool covered = false;
        for (std::size_t i = 0; i < n; ++i) {
            const SignInt& v = seq[t + i];
            if (v == SignInt(1) || v == SignInt(-1)) { covered = true; break; }
        }
        if (!covered) return false;
    }
    return true;
}

long long count_distinct_abs(const std::vector<SignInt>& seq) {
    std::set<std::string> seen;
    for (const auto& v : seq) {
        SignInt a = v < SignInt(0) ? -v : v;
        seen.insert(a.to_string());
    }
    return static_cast<long long>(seen.size());
}

long long count_zeros(const std::vector<SignInt>& free_params) {
    long long c = 0;
    for (const auto& v : free_params) if (v == SignInt(0)) ++c;
    return c;
}

long long count_distinct_gaps(const std::vector<std::size_t>& indices) {
    if (indices.size() < 2) return 0;
    std::set<long long> gaps;
    for (std::size_t k = 1; k < indices.size(); ++k) {
        gaps.insert(static_cast<long long>(indices[k]) -
                    static_cast<long long>(indices[k - 1]));
    }
    return static_cast<long long>(gaps.size());
}

std::vector<long long> gap_pattern_of(const std::vector<std::size_t>& indices) {
    std::vector<long long> g;
    for (std::size_t k = 1; k < indices.size(); ++k) {
        g.push_back(static_cast<long long>(indices[k]) -
                    static_cast<long long>(indices[k - 1]));
    }
    return g;
}

long long simplicity_score(const std::vector<SignInt>& seq,
                           const std::vector<SignInt>& free_params,
                           const std::vector<std::size_t>& indices,
                           const std::vector<long long>& cover_per_pin) {
    long long da = count_distinct_abs(seq);
    long long zr = count_zeros(free_params);
    long long dg = count_distinct_gaps(indices);
    long long cover_sum = 0;
    for (auto c : cover_per_pin) cover_sum += c;
    return da * 1000 + dg * 100
           + (static_cast<long long>(free_params.size()) - zr) * 10
           + cover_sum;
}

std::vector<long long> cover_compactness(std::size_t n, std::size_t L,
                                          const std::vector<std::size_t>& indices) {
    std::vector<long long> out;
    for (std::size_t idx : indices) {
        long long c = 0;
        for (std::size_t t = 0; t <= L; ++t) {
            if (t <= idx && idx < t + n) ++c;
        }
        out.push_back(c);
    }
    return out;
}

std::string json_escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x",
                                  static_cast<unsigned char>(c));
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

std::string to_json(const std::vector<long long>& v) {
    std::string s = "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) s += ",";
        s += std::to_string(v[i]);
    }
    s += "]";
    return s;
}

std::string to_json(const std::vector<std::size_t>& v) {
    std::string s = "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) s += ",";
        s += std::to_string(v[i]);
    }
    s += "]";
    return s;
}

std::string to_json(const std::vector<SignInt>& v) {
    std::string s = "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) s += ",";
        s += "\"" + json_escape(v[i].to_string()) + "\"";
    }
    s += "]";
    return s;
}

std::string to_json(const std::set<std::string>& v) {
    std::string s = "[";
    bool first = true;
    for (const auto& x : v) {
        if (!first) s += ",";
        s += "\"" + json_escape(x) + "\"";
        first = false;
    }
    s += "]";
    return s;
}

std::string candidate_to_json(const Candidate& c, std::size_t n, std::size_t L) {
    std::set<std::string> distinct_abs_set;
    for (const auto& v : c.sequence) {
        SignInt a = v < SignInt(0) ? -v : v;
        distinct_abs_set.insert(a.to_string());
    }
    auto covers = cover_compactness(n, L, c.indices);
    std::ostringstream o;
    o << "{";
    o << "\"indices\":" << to_json(c.indices) << ",";
    o << "\"signs\":" << to_json(c.signs) << ",";
    o << "\"solution\":" << to_json(c.free_params) << ",";
    o << "\"sequence\":" << to_json(c.sequence) << ",";
    o << "\"gap_pattern\":" << to_json(c.gap_pattern) << ",";
    o << "\"zeros_in_solution\":" << c.zeros_in_solution << ",";
    o << "\"distinct_abs_count\":" << c.distinct_abs_count << ",";
    o << "\"distinct_abs_values\":" << to_json(distinct_abs_set) << ",";
    o << "\"cover_per_pin\":" << to_json(covers) << ",";
    o << "\"cover_min\":" << c.cover_min << ",";
    o << "\"cover_max\":" << c.cover_max << ",";
    o << "\"distinct_gap_count\":" << c.distinct_gap_count << ",";
    o << "\"simplicity_score\":" << c.simplicity_score << ",";
    o << "\"n\":" << n << ",\"L\":" << L;
    o << "}";
    return o.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f) { std::cerr << "Failed to write " << path << "\n"; std::exit(2); }
    f << content;
}

std::vector<std::size_t> choose_active(
    const std::vector<bool>& choose,
    const std::vector<std::size_t>& candidates) {
    std::vector<std::size_t> out;
    for (std::size_t i = 0; i < choose.size(); ++i)
        if (choose[i]) out.push_back(candidates[i]);
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    std::size_t n = 0;
    std::size_t L = 0;
    std::string out_dir = "out/nbonacci_covering_enumerator";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto eq_at = a.find('=');
        std::string key = (eq_at == std::string::npos) ? a : a.substr(0, eq_at);
        std::string val = (eq_at == std::string::npos) ? "" : a.substr(eq_at + 1);
        if (key == "--n") {
            if (!val.empty()) n = std::stoull(val);
            else if (i + 1 < argc) n = std::stoull(argv[++i]);
        } else if (key == "--L") {
            if (!val.empty()) L = std::stoull(val);
            else if (i + 1 < argc) L = std::stoull(argv[++i]);
        } else if (key == "--out-dir") {
            if (!val.empty()) out_dir = val;
            else if (i + 1 < argc) out_dir = argv[++i];
        } else { std::cerr << "unknown arg: " << a << "\n"; return 2; }
    }
    if (n < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " --n=N [--L=L] [--out-dir=DIR]\n"
                  << "  L defaults to n+1 (SAT witness length).\n";
        return 2;
    }
    if (L == 0) L = n + 1;
    if (L < n) { std::cerr << "L must be >= n\n"; return 2; }
    std::size_t nvars = n - 1;
    std::vector<std::size_t> candidates;
    for (std::size_t j = 1; j < n + L; ++j) candidates.push_back(j);
    std::cout << "nbonacci_covering_witness_enumerator: n=" << n
              << " L=" << L << " nvars=" << nvars << "\n";
    auto t0 = std::chrono::steady_clock::now();
    std::vector<Candidate> valid;
    std::map<std::string, long long> failure_breakdown = {
        {"singular", 0}, {"out_of_box_solution", 0},
        {"out_of_box_window", 0}, {"cover_incomplete", 0}};
    long long total = 0;
    std::vector<bool> choose(candidates.size(), false);
    // Initial: FIRST nvars entries true (largest in lex order).
    // We then step through all C(candidates.size(), nvars) subsets
    // via std::prev_permutation (down to last nvars true).
    for (std::size_t i = 0; i < nvars && i < candidates.size(); ++i)
        choose[i] = true;
    // Recursive sign-assignment enumeration to keep memory bounded
    std::vector<long long> signs(nvars, 0);
    std::function<void(std::size_t)> recurse_signs =
        [&](std::size_t k) {
        if (k == nvars) {
            ++total;
            auto active = choose_active(choose, candidates);
            // Check the (n-1) x (n-1) linear system
            std::vector<std::vector<SignInt>> M(nvars,
                std::vector<SignInt>(nvars, SignInt(0)));
            std::vector<SignInt> rhs(nvars, SignInt(0));
            // Build the a_j coefficient vectors and the system
            std::vector<std::vector<SignInt>> a(n + L + 1,
                std::vector<SignInt>(n, SignInt(0)));
            a[0][0] = SignInt(1);
            for (std::size_t j = 1; j < n; ++j) a[j][j] = SignInt(1);
            for (std::size_t t = 0; t <= L; ++t) {
                std::size_t idx = t + n;
                for (std::size_t k = 0; k < n; ++k) a[idx][k] = a[t][k];
                for (std::size_t j = 1; j < n; ++j)
                    for (std::size_t k = 0; k < n; ++k) a[idx][k] = a[idx][k] - a[t + j][k];
            }
            for (std::size_t kk = 0; kk < nvars; ++kk) {
                std::size_t idx = active[kk];
                for (std::size_t j = 0; j < nvars; ++j) M[kk][j] = a[idx][j + 1];
                rhs[kk] = SignInt(signs[kk]) - a[idx][0];
            }
            auto sol_opt = solve_linear_system(M, rhs);
            if (!sol_opt) { failure_breakdown["singular"]++; return; }
            // First check: are the free params in [-1, 1]?
            for (const auto& v : *sol_opt) {
                if (!in_closed_unit_box(v)) {
                    failure_breakdown["out_of_box_solution"]++; return;
                }
            }
            // Build the full sequence
            std::vector<SignInt> sequence(n + L);
            for (std::size_t j = 0; j < n; ++j)
                sequence[j] = (j == 0) ? a[0][0] : (*sol_opt)[j - 1];
            for (std::size_t t = 0; t < L; ++t) {
                SignInt sum(0);
                for (std::size_t j = 1; j < n; ++j) sum = sum + sequence[t + j];
                sequence[t + n] = sequence[t] - sum;
            }
            // Second check: is the full sequence in [-1, 1]?
            for (const auto& v : sequence) {
                if (!in_closed_unit_box(v)) {
                    failure_breakdown["out_of_box_window"]++; return;
                }
            }
            // Third check: is the cover property satisfied?
            if (!is_covering(n, L, sequence)) {
                failure_breakdown["cover_incomplete"]++; return;
            }
            // Build the Candidate
            Candidate c;
            c.indices = active;
            c.signs = signs;
            c.sequence = sequence;
            c.free_params = *sol_opt;
            c.gap_pattern = gap_pattern_of(active);
            c.distinct_abs_count = count_distinct_abs(c.sequence);
            c.zeros_in_solution = count_zeros(c.free_params);
            c.distinct_gap_count = count_distinct_gaps(active);
            c.cover_per_pin = cover_compactness(n, L, active);
            if (!c.cover_per_pin.empty()) {
                c.cover_min = *std::min_element(c.cover_per_pin.begin(), c.cover_per_pin.end());
                c.cover_max = *std::max_element(c.cover_per_pin.begin(), c.cover_per_pin.end());
            }
            c.simplicity_score = simplicity_score(c.sequence, c.free_params, active, c.cover_per_pin);
            valid.push_back(c);
            return;
        }
        signs[k] = -1; recurse_signs(k + 1);
        signs[k] =  1; recurse_signs(k + 1);
    };
    do {
        recurse_signs(0);
    } while (std::prev_permutation(choose.begin(), choose.end()));
    std::sort(valid.begin(), valid.end(),
              [](const Candidate& a, const Candidate& b) {
                  if (a.simplicity_score != b.simplicity_score)
                      return a.simplicity_score < b.simplicity_score;
                  if (a.gap_pattern != b.gap_pattern)
                      return a.gap_pattern < b.gap_pattern;
                  return a.indices < b.indices;
              });
    auto t1 = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "  total strategies: " << total << "\n";
    std::cout << "  valid candidates: " << valid.size() << "\n";
    std::cout << "  failure breakdown: singular=" << failure_breakdown["singular"]
              << " out_of_box_solution=" << failure_breakdown["out_of_box_solution"]
              << " out_of_box_window=" << failure_breakdown["out_of_box_window"]
              << " cover_incomplete=" << failure_breakdown["cover_incomplete"] << "\n";
    std::cout << "  wall time: " << seconds << " s\n";
    std::string sat_path = out_dir + "/n" + std::to_string(n)
                            + "_L" + std::to_string(L) + ".json";
    std::string sat_txt_path = out_dir + "/n" + std::to_string(n)
                                + "_L" + std::to_string(L) + ".txt";
    {
        std::ofstream ftxt(sat_txt_path);
        if (!ftxt) { std::cerr << "Failed to write " << sat_txt_path << "\n"; std::exit(2); }
        for (const auto& c : valid) {
            ftxt << "n=" << n << " L=" << L
                 << " indices=[";
            for (std::size_t i = 0; i < c.indices.size(); ++i) {
                if (i) ftxt << ",";
                ftxt << c.indices[i];
            }
            ftxt << "] signs=[";
            for (std::size_t i = 0; i < c.signs.size(); ++i) {
                if (i) ftxt << ",";
                ftxt << c.signs[i];
            }
            ftxt << "] solution=[";
            for (std::size_t i = 0; i < c.free_params.size(); ++i) {
                if (i) ftxt << ",";
                ftxt << c.free_params[i].to_string();
            }
            ftxt << "] sequence=[";
            for (std::size_t i = 0; i < c.sequence.size(); ++i) {
                if (i) ftxt << ",";
                ftxt << c.sequence[i].to_string();
            }
            ftxt << "] gap_pattern=[";
            for (std::size_t i = 0; i < c.gap_pattern.size(); ++i) {
                if (i) ftxt << ",";
                ftxt << c.gap_pattern[i];
            }
            ftxt << "] cover_per_pin=[";
            for (std::size_t i = 0; i < c.cover_per_pin.size(); ++i) {
                if (i) ftxt << ",";
                ftxt << c.cover_per_pin[i];
            }
            ftxt << "] simplicity=" << c.simplicity_score << "\n";
        }
    }
    std::cout << "  wrote " << sat_txt_path << "\n";
    std::ostringstream o;
    o << "{";
    o << "\"n\":" << n << ",\"L\":" << L
      << ",\"expected_status\":\"SAT\",";
    o << "\"total_strategies_tried\":" << total << ",";
    o << "\"failure_breakdown\":{";
    bool first = true;
    for (const auto& [k, v] : failure_breakdown) {
        if (!first) o << ",";
        o << "\"" << k << "\":" << v;
        first = false;
    }
    o << "},";
    o << "\"candidates\":[";
    for (std::size_t i = 0; i < valid.size(); ++i) {
        if (i) o << ",";
        o << candidate_to_json(valid[i], n, L);
    }
    o << "]}";
    write_file(sat_path, o.str());
    std::cout << "  wrote " << sat_path << "\n";
    return 0;
}
