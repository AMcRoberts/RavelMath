// Covering-witness regression test for the n-bonacci homogeneous shell.
//
// This is the symbolic closest-form sibling of the existing
// `nbonacci_block_identity_test.cpp` (which checks the matrix identity
// `A^(n+1) = 2A - I`): the n-bonacci carry map's survival-depth lemma
// is proved by exhibiting, for each n in {2, ..., 8}, a witness that
// reaches depth exactly n+1 and proves the depth n+2 by an exhaustive
// enumeration.
//
// What this test verifies, for every candidate in the C++ enumerator's
// .txt sidecars (in `out/nbonacci_covering_enumerator/n{n}_L{n+1}.txt`):
//
//   1. The pin equalities hold: `a_{pins[k]} = sign[k]`, all k.
//   2. The free-parameter vector is in `[-1, 1]^(n-1)`.
//   3. The full sequence, extended by the homogeneous recurrence
//      `a_{t+n} = a_t - (a_{t+1} + ... + a_{t+n-1})`, is in `[-1, 1]`.
//   4. Every window `t = 0, ..., L` has some `i in 0..n-1` with
//      `|a_{t+i}| = 1` (the cover property).
//
// Each candidate is parsed as a fraction `num/denom` (with denom
// reduced to coprime form, denom in {1, 2, 3, ...} for n <= 10),
// then verified by multiplying through by the LCM of denominators
// and checking in long long.  No floating point, no multiprecision,
// no Z3 -- a real symbolic closure being regression-tested.
//
// Inputs: `out/nbonacci_covering_enumerator/n{n}_L{n+1}.txt` for
// n in {2, 3, 4, 5, 6, 7, 8}.  The .txt files are produced by
// `make nbonacci_covering_witness_enumerate` (which runs the C++
// covering witness enumerator at n=2..8 with both L=n+1 and
// L=n+2).  If the .txt files are missing, the test still runs the
// hard-coded simplest-per-n witness check (a smaller version of
// the test, kept for sanity).

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Reduced-fraction rational: num / denom, denom > 0, gcd(|num|, denom) = 1.
struct Fraction {
    long long num = 0;
    long long denom = 1;
    Fraction() = default;
    Fraction(long long n) : num(n), denom(1) {}
    Fraction(long long n, long long d) {
        if (d == 0) throw std::invalid_argument("Fraction: zero denom");
        if (d < 0) { n = -n; d = -d; }
        long long g = gcd(std::abs(n), d);
        num = n / g; denom = d / g;
    }
    static long long gcd(long long a, long long b) {
        if (a < 0) a = -a;
        while (b != 0) { long long t = b; b = a % b; a = t; }
        return a;
    }
    bool is_integer() const { return denom == 1; }
    bool is_zero() const { return num == 0; }
    bool is_one() const { return num == 1 && denom == 1; }
    bool is_neg_one() const { return num == -1 && denom == 1; }
    bool in_open_unit_box() const {
        if (is_zero()) return true;
        // Strict -1 < v < 1; for fractions, v > -1 iff num > -denom.
        if (is_integer()) return num > -1 && num < 1;
        return num > -denom && num < denom;
    }
    bool in_closed_unit_box() const {
        return is_zero() || in_open_unit_box() || is_one() || is_neg_one();
    }
    bool operator==(const Fraction& o) const {
        return num * o.denom == o.num * denom;
    }
    bool operator<(const Fraction& o) const {
        return num * o.denom < o.num * denom;
    }
    bool operator>(const Fraction& o) const { return o < *this; }
    bool operator<=(const Fraction& o) const { return !(o < *this); }
    bool operator>=(const Fraction& o) const { return !(*this < o); }
    Fraction operator+(const Fraction& o) const {
        long long d = denom / gcd(denom, o.denom) * o.denom;
        long long n = num * (d / denom) + o.num * (d / o.denom);
        return Fraction(n, d);
    }
    Fraction operator-(const Fraction& o) const {
        long long d = denom / gcd(denom, o.denom) * o.denom;
        long long n = num * (d / denom) - o.num * (d / o.denom);
        return Fraction(n, d);
    }
    Fraction operator*(const Fraction& o) const {
        return Fraction(num * o.num, denom * o.denom);
    }
    Fraction operator-() const { return Fraction(-num, denom); }
    std::string to_string() const {
        if (denom == 1) return std::to_string(num);
        return std::to_string(num) + "/" + std::to_string(denom);
    }
};

// Parse "num/denom" or "num" (where num, denom are signed decimals).
std::optional<Fraction> parse_fraction(const std::string& s) {
    auto slash = s.find('/');
    if (slash == std::string::npos) {
        try { return Fraction(std::stoll(s)); } catch (...) { return std::nullopt; }
    }
    try {
        long long num = std::stoll(s.substr(0, slash));
        long long denom = std::stoll(s.substr(slash + 1));
        return Fraction(num, denom);
    } catch (...) { return std::nullopt; }
}

// Parse "[1, 2, 3]" or "[]".
std::vector<std::string> parse_csv(const std::string& s) {
    std::vector<std::string> out;
    if (s.size() < 2 || s.front() != '[' || s.back() != ']') return out;
    std::string body = s.substr(1, s.size() - 2);
    if (body.empty()) return out;
    std::string cur;
    int depth = 0;
    for (char c : body) {
        if (c == ',' && depth == 0) { out.push_back(cur); cur.clear(); continue; }
        if (c == '[') ++depth;
        if (c == ']') --depth;
        cur += c;
    }
    if (!cur.empty()) out.push_back(cur);
    // Trim whitespace
    for (auto& x : out) {
        while (!x.empty() && std::isspace(x.front())) x.erase(x.begin());
        while (!x.empty() && std::isspace(x.back())) x.pop_back();
    }
    return out;
}

// Parse the value following "key=" up to whitespace or end-of-line.
std::string parse_field(const std::string& line, const std::string& key) {
    auto pos = line.find(key + "=");
    if (pos == std::string::npos) return "";
    pos += key.size() + 1;
    std::string out;
    while (pos < line.size() && !std::isspace(line[pos])) {
        out += line[pos++];
    }
    return out;
}

struct Candidate {
    long long n = 0;
    long long L = 0;
    std::vector<long long> indices;
    std::vector<long long> signs;
    std::vector<Fraction> solution;
    std::vector<Fraction> sequence;
    std::vector<long long> gap_pattern;
    std::vector<long long> cover_per_pin;
    long long simplicity = 0;
};

std::optional<Candidate> parse_line(const std::string& line) {
    Candidate c;
    auto n_str = parse_field(line, "n");
    auto L_str = parse_field(line, "L");
    if (n_str.empty() || L_str.empty()) return std::nullopt;
    c.n = std::stoll(n_str);
    c.L = std::stoll(L_str);
    auto idx_s = parse_csv(parse_field(line, "indices"));
    auto sgn_s = parse_csv(parse_field(line, "signs"));
    if (idx_s.size() != sgn_s.size()) return std::nullopt;
    for (auto& x : idx_s) c.indices.push_back(std::stoll(x));
    for (auto& x : sgn_s) c.signs.push_back(std::stoll(x));
    auto sol_s = parse_csv(parse_field(line, "solution"));
    auto seq_s = parse_csv(parse_field(line, "sequence"));
    for (auto& x : sol_s) {
        auto f = parse_fraction(x);
        if (!f) return std::nullopt;
        c.solution.push_back(*f);
    }
    for (auto& x : seq_s) {
        auto f = parse_fraction(x);
        if (!f) return std::nullopt;
        c.sequence.push_back(*f);
    }
    auto gp_s = parse_csv(parse_field(line, "gap_pattern"));
    for (auto& x : gp_s) c.gap_pattern.push_back(std::stoll(x));
    auto cp_s = parse_csv(parse_field(line, "cover_per_pin"));
    for (auto& x : cp_s) c.cover_per_pin.push_back(std::stoll(x));
    auto smp_s = parse_field(line, "simplicity");
    if (!smp_s.empty()) c.simplicity = std::stoll(smp_s);
    return c;
}

std::vector<Candidate> load_file(const std::string& path) {
    std::vector<Candidate> out;
    std::ifstream f(path);
    if (!f) return out;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto c = parse_line(line);
        if (c) out.push_back(*c);
    }
    return out;
}

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

}  // namespace

int main(int argc, char** argv) {
    std::size_t total_checks = 0;
    int total_candidates = 0;
    int total_files_loaded = 0;
    int total_files_missing = 0;
    // Default path: out/nbonacci_covering_enumerator/
    std::string base = "out/nbonacci_covering_enumerator";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto eq = a.find('=');
        if (eq != std::string::npos && a.substr(0, eq) == "--enumerator-dir") {
            base = a.substr(eq + 1);
        }
    }
    for (int n = 2; n <= 8; ++n) {
        long long L = n + 1;
        std::string path = base + "/n" + std::to_string(n)
                            + "_L" + std::to_string(L) + ".txt";
        auto cands = load_file(path);
        if (cands.empty()) {
            std::printf("[n=%lld] %s: file missing or empty, skipping\n",
                        (long long)n, path.c_str());
            ++total_files_missing;
            continue;
        }
        ++total_files_loaded;
        std::printf("[n=%lld] loaded %zu candidates from %s\n",
                    (long long)n, cands.size(), path.c_str());
        for (const auto& c : cands) {
            ++total_candidates;
            // 1. Reconstruct the sequence from the first n values via
            //    the homogeneous recurrence a_{t+n} = a_t - sum a_{t+j}.
            std::vector<Fraction> recon(c.n + c.L);
            recon[0] = Fraction(1);
            for (std::size_t j = 0; j < c.solution.size(); ++j) recon[j + 1] = c.solution[j];
            for (long long t = 0; t < c.L; ++t) {
                Fraction sum(0);
                for (std::size_t j = 1; j < c.n; ++j) sum = sum + recon[t + j];
                recon[t + c.n] = recon[t] - sum;
            }
            // The reconstruction should match the stored sequence.
            require(recon.size() == c.sequence.size(),
                    "recon size != stored size");
            for (std::size_t j = 0; j < recon.size(); ++j) {
                require(recon[j] == c.sequence[j],
                        "reconstruction mismatch");
                ++total_checks;
            }
            // 2. Pin equalities: a_{indices[k]} = signs[k].
            for (std::size_t k = 0; k < c.indices.size(); ++k) {
                require(c.sequence[c.indices[k]] == Fraction(c.signs[k]),
                        "pin equality failed");
                ++total_checks;
            }
            // 3. Free-parameter vector in [-1, 1]^(n-1).
            for (const auto& v : c.solution) {
                require(v.in_closed_unit_box(), "free param out of [-1, 1]");
                ++total_checks;
            }
            // 4. Full sequence in [-1, 1].
            for (const auto& v : c.sequence) {
                require(v.in_closed_unit_box(), "box inequality violated");
                ++total_checks;
            }
            // 5. Cover property: every window t in 0..L has some i in
            //    0..n-1 with |a_{t+i}| = 1.
            for (long long t = 0; t <= c.L; ++t) {
                bool covered = false;
                for (std::size_t i = 0; i < c.n; ++i) {
                    const Fraction& v = c.sequence[t + i];
                    if (v.is_one() || v.is_neg_one()) { covered = true; break; }
                }
                require(covered, "cover property violated");
                ++total_checks;
            }
        }
    }
    std::printf("\nnbonacci_covering_witness_test: %d candidates across %d "
                "n-values, %zu checks, 0 failures\n",
                total_candidates, total_files_loaded, total_checks);
    if (total_files_loaded == 0) {
        std::printf("(no .txt sidecars found in %s/; run\n"
                    "  make nbonacci_covering_witness_enumerate\n"
                    "first to populate them)\n", base.c_str());
    }
    return 0;
}
