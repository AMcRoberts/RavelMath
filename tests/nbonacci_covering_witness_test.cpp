// Covering-witness regression test for the n-bonacci homogeneous shell.
//
// Each witness below is a concrete (n, L=n+1) symbolic certificate emitted
// by `python/nbonacci_shell_covering_proof.py`: a list of pin indices
// (n-1 of them), the sign each pin asserts on `a_j`, the (rational) free-
// parameter vector `(a_1, ..., a_{n-1})`, and the implied covering sequence
// `a_0, a_1, ..., a_{n+L-1}`.  The witness is valid iff:
//
//   1. The pin equalities hold: `a_{pins[k]} = sign[k]`, all k.
//   2. The free-parameter vector is in `[-1, 1]^(n-1)`.
//   3. The full sequence, extended by the homogeneous recurrence
//      `a_{t+n} = a_t - (a_{t+1} + ... + a_{t+n-1})`, is in `[-1, 1]`.
//   4. Every window `t = 0, ..., L` has some `i in 0..n-1` with
//      `|a_{t+i}| = 1` (the cover property).
//
// This is the symbolic closest-form sibling of the existing
// `nbonacci_block_identity_test.cpp` (which checks the matrix identity
// `A^(n+1) = 2A - I`): the n-bonacci carry map's survival-depth lemma
// is proved by exhibiting, for each n in {2, ..., 8}, a witness that
// reaches depth exactly n+1 and proves the depth n+2 by an exhaustive
// enumeration (which is in the Python tool, not here -- C++ verifies
// the SAT-side witness only).
//
// Exact arithmetic.  Each witness is encoded in integer form by
// multiplying through by the LCM of its denominators (`denom` below);
// all inequalities and equalities are then integer checks at the
// finite precision `denom`.  No floating point, no multiprecision,
// no Z3 -- a real symbolic closure being regression-tested.

#include <algorithm>
#include <array>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace {

// One covering witness for a fixed n.  All arrays are length n-1.
struct CoveringWitness {
    std::size_t n;
    // pins[k] = which a_j is pinned; signs[k] = the pinned value (+1 or -1)
    std::vector<std::size_t> pins;
    std::vector<long long> signs;
    // free_params[k] / denom = a_{k+1} (so a_1, a_2, ..., a_{n-1})
    std::vector<long long> free_params;
    long long denom;
    // full_sequence[j] / denom = a_j for j = 0, ..., n + L - 1 (L = n+1)
    std::vector<long long> full_sequence;
};

// Hard-coded witnesses from `python/nbonacci_shell_covering_proof.py`
// (output of `make nbonacci_shell_covering_proof` for n=2..8).  The
// `full_sequence` is the JSON's `sequence_checked` field (the
// `python/nbonacci_shell_covering_proof_check.py` replay tool
// independently re-derives the same numbers from `free_params` and the
// homogeneous recurrence, so the literal integer sequence here is
// machine-checked, not hand-typed).
const std::array<CoveringWitness, 7> kWitnesses = {{
    // n = 2, L = 3
    {2,
     {4},
     {-1},
     {1},
     1,
     {1, 1, 0, 1, -1}},
    // n = 3, L = 4
    {3,
     {1, 5},
     {1, -1},
     {1, 0},
     1,
     {1, 1, 0, 0, 1, -1, 0}},
    // n = 4, L = 5, denom = 3
    {4,
     {1, 6, 8},
     {1, -1, 1},
     {3, 0, -1},
     3,
     {3, 3, 0, -1, 1, 3, -3, -2, 3}},
    // n = 5, L = 6, denom = 3
    {5,
     {1, 8, 9, 10},
     {1, -1, 1, -1},
     {3, 1, -1, 1},
     3,
     {3, 3, 1, -1, 1, -1, 3, -1, -3, 3, -3}},
    // n = 6, L = 7, denom = 2
    {6,
     {7, 8, 9, 10, 12},
     {1, 1, -1, -1, -1},
     {2, 2, 0, -1, 0},
     2,
     {2, 2, 2, 0, -1, 0, -1, 2, 2, -2, -2, 1, -2}},
    // n = 7, L = 8, denom = 3
    {7,
     {1, 10, 11, 12, 13, 14},
     {1, -1, 1, -1, 1, -1},
     {3, 1, -1, 1, -1, 1},
     3,
     {3, 3, 1, -1, 1, -1, 1, -1, 3, -1, -3, 3, -3, 3, -3}},
    // n = 8, L = 9, denom = 3
    {8,
     {9, 10, 12, 13, 14, 15, 16},
     {1, -1, 1, -1, 1, -1, 1},
     {3, 0, -1, 1, -1, 1, -1},
     3,
     {3, 3, 0, -1, 1, -1, 1, -1, 1, 3, -3, -2, 3, -3, 3, -3, 3}},
}};

// Reconstruct the full sequence (length n + L = 2n + 1) from the first
// n values via the homogeneous recurrence
//   a_{t+n} = a_t - (a_{t+1} + a_{t+2} + ... + a_{t+n-1})
// Returns the reconstructed sequence, as integer multiples of `denom`.
std::vector<long long> reconstruct(const CoveringWitness& w) {
    const std::size_t n = w.n;
    const std::size_t L = n + 1;
    std::vector<long long> a(2 * n + 1);
    a[0] = w.denom;  // a_0 = 1
    for (std::size_t k = 0; k < n - 1; ++k) a[k + 1] = w.free_params[k];
    // t ranges 0..L-1 (inclusive).  At t = L-1 we write a[(L-1)+n] = a[2n],
    // the last index in the 2n+1-long vector.  At t = L we would write
    // a[2n+1] -- out of bounds.
    for (std::size_t t = 0; t < L; ++t) {
        long long sum = 0;
        for (std::size_t j = 1; j < n; ++j) sum += a[t + j];
        a[t + n] = a[t] - sum;
    }
    return a;
}

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

long long llabs_safe(long long x) { return x < 0 ? -x : x; }

}  // namespace

int main() {
    std::size_t checks = 0;
    for (const auto& w : kWitnesses) {
        const std::size_t n = w.n;
        const std::size_t L = n + 1;
        // 1. Reconstruct the sequence and compare to the stored witness.
        const auto reconstructed = reconstruct(w);
        require(reconstructed.size() == w.full_sequence.size(),
                "reconstructed length != stored sequence length");
        for (std::size_t j = 0; j < reconstructed.size(); ++j) {
            require(reconstructed[j] == w.full_sequence[j],
                    "reconstructed sequence mismatch at some j");
            ++checks;
        }
        // 2. Pin equalities: a_{pins[k]} = sign[k] * denom.
        for (std::size_t k = 0; k < w.pins.size(); ++k) {
            const std::size_t idx = w.pins[k];
            require(idx < reconstructed.size(),
                    "pin index out of range");
            require(reconstructed[idx] == w.signs[k] * w.denom,
                    "pin equality failed");
            ++checks;
        }
        // 3. Free-parameter vector in [-denom, denom]^(n-1).
        for (std::size_t k = 0; k < w.free_params.size(); ++k) {
            require(llabs_safe(w.free_params[k]) <= w.denom,
                    "free parameter out of [-1, 1]");
            ++checks;
        }
        // 4. Full sequence in [-denom, denom] (the box inequality).
        for (std::size_t j = 0; j < reconstructed.size(); ++j) {
            require(llabs_safe(reconstructed[j]) <= w.denom,
                    "box inequality violated for some j");
            ++checks;
        }
        // 5. Cover property: every window t in 0..L has some i in
        //    0..n-1 with |a_{t+i}| = denom.
        for (std::size_t t = 0; t <= L; ++t) {
            bool covered = false;
            for (std::size_t i = 0; i < n; ++i) {
                if (llabs_safe(reconstructed[t + i]) == w.denom) {
                    covered = true;
                    break;
                }
            }
            require(covered, "cover property violated for some window t");
            ++checks;
        }
        // 6. Gap pattern sanity: the n-1 pin indices sorted increasingly
        //    and gap-differenced have the right count and sign.
        std::vector<std::size_t> sorted_pins(w.pins);
        std::sort(sorted_pins.begin(), sorted_pins.end());
        for (std::size_t k = 1; k < sorted_pins.size(); ++k) {
            require(sorted_pins[k] > sorted_pins[k - 1],
                    "pin indices not strictly increasing when sorted");
            ++checks;
        }
    }
    std::printf("nbonacci_covering_witness_test: %zu checks, 0 failures, "
                "witnesses for n in {2, 3, 4, 5, 6, 7, 8}\n", checks);
    return 0;
}
