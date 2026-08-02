// nbonacci_carry_cycle_probe.cpp
//
// Exact finite carry automaton.  For x=(x_0,...,x_{n-1}) and a signed digit
// delta, the inverse-incidence update is
//   x'=(x_1,...,x_{n-1}, x_0+delta-sum_{k>0} x_k).
//
// We enumerate a coefficient box, install all three digit transitions, and
// repeatedly remove vertices with zero in- or out-degree.  The survivors are
// exactly the vertices lying on directed cycles.  This is a stronger finite
// probe than the labelled arithmetic hull: it permits every digit at every
// step and uses no beta-window or face constraints.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Options {
    int n = 3;
    int bound = 2;
    bool print_witness = false;
};

std::size_t power(std::size_t base, int exponent) {
    std::size_t result = 1;
    for (int k = 0; k < exponent; ++k) result *= base;
    return result;
}

std::vector<int> decode(std::size_t code, int n, int bound, std::size_t base) {
    std::vector<int> x(n);
    for (int k = 0; k < n; ++k) {
        x[k] = static_cast<int>(code % base) - bound;
        code /= base;
    }
    return x;
}

std::size_t encode(const std::vector<int>& x, int bound, std::size_t base) {
    std::size_t code = 0;
    std::size_t place = 1;
    for (const int value : x) {
        code += static_cast<std::size_t>(value + bound) * place;
        place *= base;
    }
    return code;
}

bool is_nonternary(std::size_t code, int n, int bound, std::size_t base) {
    const auto x = decode(code, n, bound, base);
    for (const int value : x)
        if (std::abs(value) >= 2) return true;
    return false;
}

int run(const Options& options) {
    if (options.n < 2 || options.bound < 1)
        throw std::invalid_argument("require n>=2 and bound>=1");
    const std::size_t base = static_cast<std::size_t>(2 * options.bound + 1);
    const std::size_t states = power(base, options.n);
    if (states > 3000000)
        throw std::invalid_argument("box too large; use a smaller n or bound");

    std::vector<std::vector<std::uint32_t>> out(states), in(states);
    std::size_t edges = 0;
    for (std::size_t code = 0; code < states; ++code) {
        const auto x = decode(code, options.n, options.bound, base);
        int tail = x[0];
        for (int k = 1; k < options.n; ++k) tail -= x[k];
        for (int delta = -1; delta <= 1; ++delta) {
            auto next = x;
            for (int k = 0; k + 1 < options.n; ++k) next[k] = x[k + 1];
            next.back() = tail + delta;
            if (next.back() < -options.bound || next.back() > options.bound)
                continue;
            const auto destination = encode(next, options.bound, base);
            out[code].push_back(static_cast<std::uint32_t>(destination));
            in[destination].push_back(static_cast<std::uint32_t>(code));
            ++edges;
        }
    }

    std::vector<int> indegree(states), outdegree(states);
    std::deque<std::size_t> queue;
    for (std::size_t code = 0; code < states; ++code) {
        indegree[code] = static_cast<int>(in[code].size());
        outdegree[code] = static_cast<int>(out[code].size());
        if (indegree[code] == 0 || outdegree[code] == 0) queue.push_back(code);
    }
    std::vector<bool> removed(states, false);
    while (!queue.empty()) {
        const auto code = queue.front();
        queue.pop_front();
        if (removed[code]) continue;
        removed[code] = true;
        for (const auto destination : out[code]) {
            if (removed[destination]) continue;
            if (--indegree[destination] == 0) queue.push_back(destination);
        }
        for (const auto source : in[code]) {
            if (removed[source]) continue;
            if (--outdegree[source] == 0) queue.push_back(source);
        }
    }

    std::size_t cyclic = 0, nonternary_cyclic = 0;
    std::size_t first_nonternary = states;
    for (std::size_t code = 0; code < states; ++code) {
        if (removed[code]) continue;
        ++cyclic;
        if (is_nonternary(code, options.n, options.bound, base)) {
            ++nonternary_cyclic;
            if (first_nonternary == states) first_nonternary = code;
        }
    }
    std::printf(
        "carry cycle probe: n=%d bound=%d states=%zu edges=%zu "
        "cyclic=%zu nonternary_cyclic=%zu\n",
        options.n, options.bound, states, edges, cyclic, nonternary_cyclic);
    if (options.print_witness && first_nonternary != states) {
        const auto witness = decode(first_nonternary, options.n,
                                    options.bound, base);
        std::printf("first_nonternary_witness=");
        for (std::size_t k = 0; k < witness.size(); ++k)
            std::printf("%s%d", k == 0 ? "" : ",", witness[k]);
        std::printf("\n");
    }
    return nonternary_cyclic == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    for (int arg = 1; arg < argc; ++arg) {
        const std::string value(argv[arg]);
        if (value.rfind("--n=", 0) == 0)
            options.n = std::stoi(value.substr(4));
        else if (value.rfind("--bound=", 0) == 0)
            options.bound = std::stoi(value.substr(8));
        else if (value == "--print-witness")
            options.print_witness = true;
        else {
            std::fprintf(stderr, "usage: %s --n=N --bound=B\n", argv[0]);
            return 2;
        }
    }
    try {
        return run(options);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "carry cycle probe: %s\n", error.what());
        return 2;
    }
}
