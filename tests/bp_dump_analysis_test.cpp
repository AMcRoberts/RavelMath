#include "ravel/bp_dump_analysis.hpp"

#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace ravel;

static int tests = 0, failed = 0;
#define CHECK(expr, label) do { ++tests; if (!(expr)) { \
    ++failed; std::printf("[FAIL] %s\n", label); \
} else std::printf("[ok]   %s\n", label); } while (0)

int main() {
    const std::string fixture =
        "# n=3 recurrent_core_size=2\n"
        "STATES\n"
        "S 0 01 10 2\n"
        "S 1 0 1 1\n"
        "EDGES\n"
        "E 0 0 1 4 2\n"
        "E 0 1 2 4 1\n"
        "E 0 0 3 4 2\n"
        "E 1 0 0 1 2\n";
    std::istringstream input(fixture);
    auto result = analyze_bp_dump(input);
    CHECK(result.n == 3 && result.core_size == 2,
          "metadata parsed");
    CHECK(result.edge_records == 4 && result.extra_rows == 1,
          "sparse multiplicities recovered");
    CHECK(result.correction_nilpotent && result.nilpotency_index == 1,
          "zero reduced correction is nilpotent of index one");
    CHECK(result.correction_lower_triangular_by_length,
          "correction is lower triangular by word length");
    CHECK(result.branching_states == 1,
          "branching state counted");
    CHECK(result.chunk_counts_are_powers_of_two,
          "chunk count power-of-two property");
    CHECK(result.position_zero_is_transient,
          "position zero absent from recurrent edges");
    CHECK(result.designated_is_max_v2_position,
          "designated successor is the maximum-v2 position");

    bool threw = false;
    try {
        std::istringstream bad(fixture);
        BpDumpLimits limits;
        limits.max_edges = 2;
        (void)analyze_bp_dump(bad, limits);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw, "edge cap is enforced");

    threw = false;
    try {
        std::istringstream bad(
            "# n=3 recurrent_core_size=2\nSTATES\nS 0 0 1 1\nEDGES\n");
        (void)analyze_bp_dump(bad);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw, "incomplete state table is rejected");

    std::printf("%d tests run, %d failed.\n", tests, failed);
    return failed ? 1 : 0;
}
