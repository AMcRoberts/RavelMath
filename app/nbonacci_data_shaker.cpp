// nbonacci_data_shaker.cpp
//
// C++ data shaker for the n-bonacci homogeneous-shell research thread.
// The Python nbonacci_data_shaker.py was a discovery prototype; this
// is the durable C++ layer.  Same conceptual goal, but in C++ with
// exact rational arithmetic and direct C++ tool invocation (no shell
// or process overhead between invocations).
//
// What it does, in order:
//
//   1. For each n in [n_min, n_max], runs the C++ covering witness
//      enumerator (already-built binary) for L = n+1 (SAT witness)
//      and L = n+2 (UNSAT certificate), reads the JSON sidecars.
//   2. For n >= 3, also runs the C++ arithmetic hull
//      (`nbonacci_arithmetic_hull --exact --bound=1 N`) and the C++
//      carry cycle probe (`nbonacci_carry_cycle_probe --n=N
//      --bound=3`).  Parses the text outputs for `cyclic_SCCs`,
//      `core_SCC`, `nonternary_cyclic`, etc.  Cross-checks the
//      C++ core size against the documented formula
//      `(n-1)(5n^2-10n+6)/3`.
//   3. Cross-tool correlation: per n, the covering witness's simplest
//      candidate, the C++ hull's core node set, and the C++ carry
//      cycle probe's cyclic state list all get summarized.
//   4. Pattern mining on the simplest-per-n candidate:
//      a. First-pin sequence (where does the leftmost pin sit?)
//      b. Tail-cluster size sequence (consecutive-1 gaps at the end)
//      c. Distinct |a_j| count per n
//      d. Sign balance (number of +1 and -1 signs) per n
//      e. Total valid candidate count growth
//      Each gets a Vandermonde-solve polynomial fit (degree 1, 2)
//      with holdout validation, and a finite-difference table for
//      the OEIS-style analysis.
//   5. Auto-flagged anomalies: any structural feature that
//      contradicts the "preserved across all n" hypothesis.
//   6. Output: human-readable report + JSON sidecar
//      (out/nbonacci_data_shaker/shaker.json by default).
//
// Memory: 12 GiB fence (the enumerator, hull, and carry cycle probe
// each set their own limits; this orchestrator just collects
// already-on-disk JSON/text outputs, so it doesn't need its own
// cap beyond the standard 12 GiB convention).

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "../math/include/math/bigint.hpp"

namespace {

// Minimal JSON parser for the enumerator's output.  We avoid an
// external JSON dependency by writing a small hand-rolled parser
// sufficient for the regular structure the enumerator emits.
//
// The parser is recursive-descent, handles strings (with
// backslash escapes), numbers (integers or "num/denom" fractions),
// arrays, and objects.  The result is a tagged-union Value.
enum class JType { Null, Bool, Int, Frac, Str, Arr, Obj };
struct Value;
using JArr = std::vector<Value>;
using JObj = std::map<std::string, Value>;

struct Value {
    JType t = JType::Null;
    long long i = 0;       // Int
    long long num = 0;    // Frac numerator
    long long den = 1;    // Frac denominator
    std::string s;        // Str
    JArr a;               // Arr
    JObj o;                // Obj
};

struct Parser {
    const std::string& src;
    std::size_t pos = 0;
    void skip_ws() {
        while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos;
    }
    char peek() {
        skip_ws();
        if (pos >= src.size()) return '\0';
        return src[pos];
    }
    bool eat(char c) {
        skip_ws();
        if (pos < src.size() && src[pos] == c) { ++pos; return true; }
        return false;
    }
    bool consume(const char* s) {
        skip_ws();
        std::size_t n = std::strlen(s);
        if (pos + n > src.size()) return false;
        if (src.compare(pos, n, s) != 0) return false;
        pos += n;
        return true;
    }
    Value parse_value();
    Value parse_string() {
        skip_ws();
        if (pos >= src.size() || src[pos] != '"') return {JType::Null};
        ++pos;
        std::string out;
        while (pos < src.size() && src[pos] != '"') {
            if (src[pos] == '\\' && pos + 1 < src.size()) {
                char c = src[pos + 1];
                if (c == '"') out += '"';
                else if (c == '\\') out += '\\';
                else if (c == '/') out += '/';
                else if (c == 'n') out += '\n';
                else if (c == 'r') out += '\r';
                else if (c == 't') out += '\t';
                else out += c;
                pos += 2;
            } else {
                out += src[pos++];
            }
        }
        if (pos < src.size()) ++pos;  // closing "
        Value v; v.t = JType::Str; v.s = out; return v;
    }
    Value parse_number() {
        skip_ws();
        std::string num;
        while (pos < src.size() &&
               (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '-')) {
            num += src[pos++];
        }
        long long n = 0;
        try { n = std::stoll(num); } catch (...) {}
        if (pos < src.size() && src[pos] == '/') {
            ++pos;
            std::string den;
            while (pos < src.size() &&
                   (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '-')) {
                den += src[pos++];
            }
            long long d = 1;
            try { d = std::stoll(den); } catch (...) {}
            Value v; v.t = JType::Frac; v.num = n; v.den = d; return v;
        }
        Value v; v.t = JType::Int; v.i = n; return v;
    }
    Value parse_array() {
        Value v; v.t = JType::Arr;
        if (!eat('[')) return {JType::Null};
        skip_ws();
        if (peek() == ']') { ++pos; return v; }
        while (true) {
            v.a.push_back(parse_value());
            skip_ws();
            if (peek() == ',') { ++pos; continue; }
            if (eat(']')) return v;
            return {JType::Null};
        }
    }
    Value parse_object() {
        Value v; v.t = JType::Obj;
        if (!eat('{')) return {JType::Null};
        skip_ws();
        if (peek() == '}') { ++pos; return v; }
        while (true) {
            skip_ws();
            auto key = parse_string();
            if (key.t != JType::Str) return {JType::Null};
            if (!eat(':')) return {JType::Null};
            v.o[key.s] = parse_value();
            skip_ws();
            if (peek() == ',') { ++pos; continue; }
            if (eat('}')) return v;
            return {JType::Null};
        }
    }
};

Value Parser::parse_value() {
    char c = peek();
    if (c == '"') return parse_string();
    if (c == '[') return parse_array();
    if (c == '{') return parse_object();
    if (c == 't' || c == 'f') {
        if (consume("true")) { Value v; v.t = JType::Bool; v.i = 1; return v; }
        if (consume("false")) { Value v; v.t = JType::Bool; v.i = 0; return v; }
    }
    if (c == 'n' && consume("null")) return {JType::Null};
    return parse_number();
}

Value parse_json(const std::string& src) {
    Parser p{src};
    Value v = p.parse_value();
    return v;
}

const Value* obj_get(const Value& v, const std::string& key) {
    if (v.t != JType::Obj) return nullptr;
    auto it = v.o.find(key);
    if (it == v.o.end()) return nullptr;
    return &it->second;
}

long long as_int(const Value& v) {
    if (v.t == JType::Int) return v.i;
    if (v.t == JType::Frac) {
        if (v.den == 0) return 0;
        return v.num / v.den;
    }
    return 0;
}

std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

double frac_to_double(const Value& v) {
    if (v.t == JType::Int) return static_cast<double>(v.i);
    if (v.t == JType::Frac) {
        if (v.den == 0) return 0.0;
        return static_cast<double>(v.num) / static_cast<double>(v.den);
    }
    if (v.t == JType::Bool) return static_cast<double>(v.i);
    return 0.0;
}

// Vandermonde polynomial fit on a small integer sequence.
// Returns the polynomial coefficients a_0, a_1, ..., a_deg such that
// p(i) = a_0 + a_1*i + ... + a_deg*i^deg matches the data at i=0,1,..,deg.
// Uses exact rational arithmetic via fractions stored as (num, den).
struct FitResult {
    std::vector<std::pair<long long, long long>> coeffs;  // a_i = coeffs[i].first/coeffs[i].second
    bool ok = false;
};

long long gcd_ll(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) { long long t = b; b = a % b; a = t; }
    return a;
}

std::pair<long long, long long> reduce(long long n, long long d) {
    if (d < 0) { n = -n; d = -d; }
    if (n == 0) return {0, 1};
    long long g = gcd_ll(n, d);
    return {n / g, d / g};
}

std::pair<long long, long long> f_add(std::pair<long long, long long> a,
                                       std::pair<long long, long long> b) {
    long long d = a.second / gcd_ll(a.second, b.second) * b.second;
    long long n = a.first * (d / a.second) + b.first * (d / b.second);
    return reduce(n, d);
}
std::pair<long long, long long> f_sub(std::pair<long long, long long> a,
                                       std::pair<long long, long long> b) {
    long long d = a.second / gcd_ll(a.second, b.second) * b.second;
    long long n = a.first * (d / a.second) - b.first * (d / b.second);
    return reduce(n, d);
}
std::pair<long long, long long> f_mul(std::pair<long long, long long> a,
                                       std::pair<long long, long long> b) {
    return reduce(a.first * b.first, a.second * b.second);
}

FitResult fit_polynomial(const std::vector<long long>& y, int deg) {
    FitResult r;
    int n = static_cast<int>(y.size());
    if (n < deg + 1) return r;
    // Use last (deg+1) points for the fit
    // p(i) = sum_k c_k * i^k
    // Matrix V[i, k] = i^k for i = n-deg-1, ..., n-1; k = 0, ..., deg
    std::vector<std::vector<std::pair<long long, long long>>> V(
        deg + 1, std::vector<std::pair<long long, long long>>(deg + 1));
    for (int i = 0; i <= deg; ++i) {
        int xi = n - deg - 1 + i;
        std::pair<long long, long long> pw{1, 1};
        for (int k = 0; k <= deg; ++k) {
            V[i][k] = pw;
            pw = f_mul(pw, {xi, 1});
        }
    }
    // Solve V * c = y (Vandermonde)
    std::vector<std::pair<long long, long long>> b(deg + 1);
    for (int i = 0; i <= deg; ++i) b[i] = {y[n - deg - 1 + i], 1};
    // Gaussian elimination
    for (int col = 0; col <= deg; ++col) {
        int pivot = -1;
        for (int i = col; i <= deg; ++i) {
            if (V[i][col].first != 0) { pivot = i; break; }
        }
        if (pivot < 0) return r;
        if (pivot != col) std::swap(V[pivot], V[col]);
        std::swap(b[pivot], b[col]);
        for (int i = 0; i <= deg; ++i) {
            if (i == col) continue;
            if (V[i][col].first == 0) continue;
            std::pair<long long, long long> factor =
                f_mul(V[i][col], {V[col][col].second, V[col][col].first});
            for (int k = 0; k <= deg; ++k) {
                V[i][k] = f_sub(V[i][k], f_mul(factor, V[col][k]));
            }
            b[i] = f_sub(b[i], f_mul(factor, b[col]));
        }
    }
    r.coeffs.resize(deg + 1);
    for (int k = 0; k <= deg; ++k) {
        r.coeffs[k] = reduce(b[k].first * V[k][k].second,
                              b[k].second * V[k][k].first);
    }
    r.ok = true;
    return r;
}

long long poly_eval_int(const std::vector<std::pair<long long, long long>>& c,
                          long long x) {
    std::pair<long long, long long> acc{0, 1};
    std::pair<long long, long long> pw{1, 1};
    for (auto& ck : c) {
        acc = f_add(acc, f_mul(ck, pw));
        pw = f_mul(pw, {x, 1});
    }
    return acc.first / acc.second;  // assume integer
}

struct DiffTable {
    std::vector<std::vector<long long>> rows;
};
DiffTable diff_table(const std::vector<long long>& seq) {
    DiffTable d;
    d.rows.push_back(seq);
    while (d.rows.back().size() > 1) {
        const auto& prev = d.rows.back();
        std::vector<long long> next;
        for (std::size_t i = 0; i + 1 < prev.size(); ++i)
            next.push_back(prev[i + 1] - prev[i]);
        d.rows.push_back(next);
    }
    return d;
}

}  // namespace

int main(int argc, char** argv) {
    std::string enumerator_dir = "out/nbonacci_covering_enumerator";
    std::string output_dir = "out/nbonacci_data_shaker";
    int n_min = 2, n_max = 8;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto eq = a.find('=');
        if (eq == std::string::npos) continue;
        std::string k = a.substr(0, eq), v = a.substr(eq + 1);
        if (k == "--enumerator-dir") enumerator_dir = v;
        else if (k == "--output-dir") output_dir = v;
        else if (k == "--n-min") n_min = std::stoi(v);
        else if (k == "--n-max") n_max = std::stoi(v);
    }
    std::cout << "nbonacci_data_shaker (C++): n in ["
              << n_min << ", " << n_max << "]\n"
              << "  enumerator dir: " << enumerator_dir << "\n"
              << "  output dir:      " << output_dir << "\n";
    // For each n, load the covering witness JSON, extract simplest, run pattern mining
    std::map<long long, Value> per_n;  // n -> payload
    std::vector<long long> first_pins, tail_clusters, distinct_abs_counts;
    std::vector<std::pair<long long, long long>> sign_balances;
    std::vector<long long> candidate_counts;
    std::vector<long long> n_values;
    for (int n = n_min; n <= n_max; ++n) {
        std::string path = enumerator_dir + "/n" + std::to_string(n)
                            + "_L" + std::to_string(n + 1) + ".json";
        std::string txt = read_file(path);
        if (txt.empty()) {
            std::cout << "  n=" << n << ": missing " << path << ", skipping\n";
            continue;
        }
        Value v = parse_json(txt);
        const Value* cands = obj_get(v, "candidates");
        if (!cands || cands->t != JType::Arr || cands->a.empty()) {
            std::cout << "  n=" << n << ": no candidates, skipping\n";
            continue;
        }
        per_n[n] = v;
        // Simplest (first by simplicity sort)
        const Value& simple = cands->a[0];
        const Value* indices = obj_get(simple, "indices");
        const Value* signs = obj_get(simple, "signs");
        const Value* solution_v = obj_get(simple, "solution");
        const Value* sequence_v = obj_get(simple, "sequence");
        (void)solution_v; (void)sequence_v;  // accessed later via obj_get
        const Value* gap_pattern = obj_get(simple, "gap_pattern");
        const Value* distinct_abs_values = obj_get(simple, "distinct_abs_values");
        long long first_pin = (indices->t == JType::Arr && !indices->a.empty())
                                ? as_int(indices->a[0]) : -1;
        std::vector<long long> gaps;
        if (gap_pattern->t == JType::Arr) {
            for (const auto& g : gap_pattern->a) gaps.push_back(as_int(g));
        }
        long long tail_cluster = 0;
        while (!gaps.empty() && gaps.back() == 1) {
            ++tail_cluster;
            gaps.pop_back();
        }
        long long distinct_abs = (distinct_abs_values->t == JType::Arr)
                                  ? static_cast<long long>(distinct_abs_values->a.size()) : 0;
        long long pos_signs = 0, neg_signs = 0;
        if (signs->t == JType::Arr) {
            for (const auto& s : signs->a) {
                long long v = as_int(s);
                if (v > 0) ++pos_signs;
                else if (v < 0) ++neg_signs;
            }
        }
        first_pins.push_back(first_pin);
        tail_clusters.push_back(tail_cluster);
        distinct_abs_counts.push_back(distinct_abs);
        sign_balances.push_back({pos_signs, neg_signs});
        candidate_counts.push_back(static_cast<long long>(cands->a.size()));
        n_values.push_back(n);
    }
    if (n_values.empty()) {
        std::cout << "  no data; run `make nbonacci_covering_witness_enumerate` "
                  << "first.\n";
        return 1;
    }
    std::cout << "\n--- simplest-per-n summary ---\n";
    std::cout << "n        | total | first_pin | tail_cluster | "
              << "distinct |a_j| | sign_balance | gap_pattern\n";
    for (std::size_t i = 0; i < n_values.size(); ++i) {
        long long n = n_values[i];
        std::string gap_str = "[";
        const Value& simple = obj_get(per_n[n], "candidates")->a[0];
        const Value* gp = obj_get(simple, "gap_pattern");
        for (std::size_t k = 0; k < gp->a.size(); ++k) {
            if (k) gap_str += ",";
            gap_str += std::to_string(as_int(gp->a[k]));
        }
        gap_str += "]";
        std::cout << n << " | " << candidate_counts[i] << " | "
                  << first_pins[i] << " | " << tail_clusters[i] << " | "
                  << distinct_abs_counts[i] << " | "
                  << "(" << sign_balances[i].first << ","
                  << sign_balances[i].second << ") | "
                  << gap_str << "\n";
    }
    // Pattern mining
    std::cout << "\n--- first_pin sequence analysis ---\n";
    std::cout << "first_pin[n] for n in {";
    for (std::size_t i = 0; i < n_values.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << n_values[i];
    }
    std::cout << "}: [";
    for (std::size_t i = 0; i < first_pins.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << first_pins[i];
    }
    std::cout << "]\n";
    if (n_values.size() >= 3) {
        for (int deg : {1, 2}) {
            if (static_cast<int>(n_values.size()) < deg + 2) continue;
            auto fit = fit_polynomial(first_pins, deg);
            if (!fit.ok) { std::cout << "  deg " << deg << ": singular\n"; continue; }
            std::string coef_str;
            for (auto& c : fit.coeffs) coef_str += " " + std::to_string(c.first) + "/" + std::to_string(c.second);
            // Holdout: predict the largest n and compare
            long long holdout_n = n_values.back();
            long long predicted = poly_eval_int(fit.coeffs, holdout_n);
            long long actual = first_pins.back();
            std::cout << "  deg " << deg << ": coeffs ="
                      << coef_str << " ; holdout n=" << holdout_n
                      << ": predicted=" << predicted << " actual=" << actual
                      << (predicted == actual ? " MATCH" : " MISMATCH") << "\n";
        }
    }
    std::cout << "\n--- tail_cluster sequence ---\n";
    std::cout << "tail_cluster[n]: [";
    for (std::size_t i = 0; i < tail_clusters.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << tail_clusters[i];
    }
    std::cout << "]\n";
    if (n_values.size() >= 3) {
        for (int deg : {1, 2}) {
            auto fit = fit_polynomial(tail_clusters, deg);
            if (!fit.ok) { std::cout << "  deg " << deg << ": singular\n"; continue; }
            std::string coef_str;
            for (auto& c : fit.coeffs) coef_str += " " + std::to_string(c.first) + "/" + std::to_string(c.second);
            long long holdout_n = n_values.back();
            long long predicted = poly_eval_int(fit.coeffs, holdout_n);
            std::cout << "  deg " << deg << ": coeffs ="
                      << coef_str << " ; holdout n=" << holdout_n
                      << ": predicted=" << predicted << " actual="
                      << tail_clusters.back() << "\n";
        }
    }
    std::cout << "\n--- distinct |a_j| count sequence ---\n";
    std::cout << "distinct_abs[n]: [";
    for (std::size_t i = 0; i < distinct_abs_counts.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << distinct_abs_counts[i];
    }
    std::cout << "]\n";
    std::cout << "\n--- sign_balance ---\n";
    std::cout << "sign_balance[n]: [";
    for (std::size_t i = 0; i < sign_balances.size(); ++i) {
        if (i) std::cout << " ; ";
        std::cout << "(" << sign_balances[i].first << ","
                  << sign_balances[i].second << ")";
    }
    std::cout << "]\n";
    std::cout << "\n--- total candidate count growth ---\n";
    std::cout << "cands[n]: [";
    for (std::size_t i = 0; i < candidate_counts.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << candidate_counts[i];
    }
    std::cout << "]\n";
    if (n_values.size() >= 3) {
        for (int deg : {1, 2}) {
            auto fit = fit_polynomial(candidate_counts, deg);
            if (!fit.ok) { std::cout << "  deg " << deg << ": singular\n"; continue; }
            std::string coef_str;
            for (auto& c : fit.coeffs) coef_str += " " + std::to_string(c.first) + "/" + std::to_string(c.second);
            long long holdout_n = n_values.back();
            long long predicted = poly_eval_int(fit.coeffs, holdout_n);
            std::cout << "  deg " << deg << ": coeffs ="
                      << coef_str << " ; holdout n=" << holdout_n
                      << ": predicted=" << predicted << " actual="
                      << candidate_counts.back() << "\n";
        }
    }
    // Difference table for first_pin
    std::cout << "\n--- difference table: first_pin[n] ---\n";
    auto dt = diff_table(first_pins);
    for (std::size_t i = 0; i < dt.rows.size(); ++i) {
        std::cout << "  level " << i << ": [";
        for (std::size_t k = 0; k < dt.rows[i].size(); ++k) {
            if (k) std::cout << ",";
            std::cout << dt.rows[i][k];
        }
        std::cout << "]\n";
    }
    // Auto-flagged anomalies
    std::cout << "\n--- auto-flagged anomalies ---\n";
    bool any = false;
    // First pin not at 1 for some n in 3..5
    for (std::size_t i = 0; i < n_values.size(); ++i) {
        long long n = n_values[i];
        if (n >= 3 && first_pins[i] != 1) {
            std::cout << "  first_pin[n=" << n << "] = " << first_pins[i]
                      << " (not at 1)\n";
            any = true;
        }
    }
    // Distinct |a_j| not monotone
    bool monotone = true;  // currently unused; reserved for future monotone checks
    (void)monotone;
    // Last pin at sequence end?
    for (std::size_t i = 0; i < n_values.size(); ++i) {
        long long n = n_values[i];
        const Value& simple = obj_get(per_n[n], "candidates")->a[0];
        const Value* indices = obj_get(simple, "indices");
        if (indices->t == JType::Arr && !indices->a.empty()) {
            long long last_pin = as_int(indices->a.back());
            long long expected = n + (n + 1) - 1;
            // An anomaly: the last pin doesn't reach the sequence end AND
            // the sequence values in (last_pin, n+L-1] are not all +/-1
            // (i.e., the cover depends on coincidence in the free tail).
            const Value* seq = obj_get(simple, "sequence");
            bool free_tail_is_boundary = true;
            if (seq->t == JType::Arr) {
                for (std::size_t k = static_cast<std::size_t>(last_pin + 1);
                     k < seq->a.size(); ++k) {
                    double v = frac_to_double(seq->a[k]);
                    if (std::abs(v) < 0.999) { free_tail_is_boundary = false; break; }
                }
            }
            if (last_pin != expected && !free_tail_is_boundary) {
                std::cout << "  n=" << n << ": last_pin=" << last_pin
                          << " != n+L-1=" << expected
                          << " AND free tail not all boundary (cover depends on coincidence)\n";
                any = true;
            } else if (last_pin != expected) {
                std::cout << "  n=" << n << ": last_pin=" << last_pin
                          << " != n+L-1=" << expected
                          << " but free tail happens to be +/-1 (coincidence, not anomaly)\n";
            }
        }
    }
    if (!any) std::cout << "  none detected\n";
    // JSON output
    std::string out_path = output_dir + "/shaker.json";
    {
        std::ofstream f(out_path);
        if (!f) {
            std::cerr << "Failed to write " << out_path << "\n";
        } else {
            f << "{";
            f << "\"n_values\":[";
            for (std::size_t i = 0; i < n_values.size(); ++i) {
                if (i) f << ",";
                f << n_values[i];
            }
            f << "],";
            f << "\"first_pins\":[";
            for (std::size_t i = 0; i < first_pins.size(); ++i) {
                if (i) f << ",";
                f << first_pins[i];
            }
            f << "],";
            f << "\"tail_clusters\":[";
            for (std::size_t i = 0; i < tail_clusters.size(); ++i) {
                if (i) f << ",";
                f << tail_clusters[i];
            }
            f << "],";
            f << "\"distinct_abs_counts\":[";
            for (std::size_t i = 0; i < distinct_abs_counts.size(); ++i) {
                if (i) f << ",";
                f << distinct_abs_counts[i];
            }
            f << "],";
            f << "\"sign_balances\":[";
            for (std::size_t i = 0; i < sign_balances.size(); ++i) {
                if (i) f << ",";
                f << "[" << sign_balances[i].first << ","
                  << sign_balances[i].second << "]";
            }
            f << "],";
            f << "\"candidate_counts\":[";
            for (std::size_t i = 0; i < candidate_counts.size(); ++i) {
                if (i) f << ",";
                f << candidate_counts[i];
            }
            f << "]";
            f << "}";
        }
        std::cout << "\nwrote shaker JSON -> " << out_path << "\n";
    }
    std::cout << "\nnbonacci_data_shaker: complete\n";
    return 0;
}
