// tests/weighted_digraph_file_test.cpp
//
// Round-trip tests for WeightedDigraph::from_dense_file (the streaming
// GBMT0002-format loader).  We write a small adjacency matrix to a
// temp file using the same on-disk format the cache uses, then load
// it back via both the in-memory `from_dense` path (control) and the
// streaming `from_dense_file` path.  The two resulting digraphs
// must be identical edge-by-edge.
//
// Why this matters:  the streaming loader is the only way we can
// re-use large G_B caches (N=10K+) without blowing the memory
// policy's 70% ceiling, so its correctness for the standard case
// (an exact-round-trip-load) is a hard prerequisite for the larger
// Pisot 4-letter audits coming out of items A1 / A2.

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <random>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "ravel/graph_divisor.hpp"

using namespace ravel;

static int n_pass = 0, n_fail = 0;

#define EXPECT(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; std::fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

// FNV-1a 64-bit, must match the save side exactly.
static uint64_t fnv1a_64(const void* data, std::size_t n,
                         uint64_t h = 0xcbf29ce484222325ULL) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    for (std::size_t i = 0; i < n; ++i) {
        h ^= p[i];
        h *= 0x100000001b3ULL;
    }
    return h;
}

// Minimal GBMT0002 writer (kept here for the round-trip test, not in
// production code).  Writes magic + N + N² + rows + footer.
static bool write_cache_file(const std::string& path,
                            const std::vector<std::vector<long long>>& M) {
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return false;
    const char magic[8] = {'G','B','M','T','0','0','0','2'};
    auto write_all = [&](const void* p, std::size_t n) {
        const uint8_t* q = static_cast<const uint8_t*>(p);
        while (n > 0) {
            ssize_t w = ::write(fd, q, n);
            if (w <= 0) return false;
            q += w; n -= static_cast<std::size_t>(w);
        }
        return true;
    };
    auto feed = [&](const void* p, std::size_t n, uint64_t& h) {
        if (write_all(p, n)) { h = fnv1a_64(p, n, h); return true; }
        return false;
    };

    bool ok = true;
    uint64_t h = 0xcbf29ce484222325ULL;
    std::size_t N = M.size();
    uint64_t nn = static_cast<uint64_t>(N) * static_cast<uint64_t>(N);
    if (ok) ok = feed(magic, 8, h);
    if (ok) ok = feed(&N, sizeof(N), h);
    if (ok) ok = feed(&nn, sizeof(nn), h);
    for (std::size_t i = 0; i < N && ok; ++i) {
        const auto& row = M[i];
        if (row.size() != N) { ok = false; break; }
        ok = feed(row.data(), sizeof(long long) * N, h);
    }
    if (ok) ok = write_all(&h, sizeof(h));
    ::close(fd);
    return ok;
}

// Compare two WeightedDigraphs edge-by-edge (sorted tuple equality);
// ignores in/out_adj differentiation -- only the underlying (u,v,w)
// multisets need to match, since streaming and in-memory could
// differ on the order add_edge() fires within a row.
static bool digraphs_equal(const WeightedDigraph& a, const WeightedDigraph& b) {
    if (a.n != b.n) return false;
    auto collect = [](const WeightedDigraph& g) {
        std::vector<std::tuple<long long, long long, long long>> v;
        for (std::size_t u = 0; u < g.n; ++u) {
            for (auto& [t, w] : g.out_adj[u]) {
                v.emplace_back(static_cast<long long>(u),
                               static_cast<long long>(t), w);
            }
        }
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
        return v;
    };
    auto va = collect(a);
    auto vb = collect(b);
    if (va.size() != vb.size()) return false;
    return va == vb;
}

int main() {
    std::printf("== WeightedDigraph::from_dense_file round-trip ==\n");
    std::fflush(stdout);

    // ---- (1) small hand-crafted matrix ----
    {
        std::vector<std::vector<long long>> M = {
            {0, 1, 0, 2, 0},
            {0, 0, 0, 0, 3},
            {4, 0, 0, 0, 0},
            {0, 5, 0, 0, 6},
            {1, 0, 0, 7, 0}};
        std::string path = "/tmp/wd_roundtrip_small.gbm";
        if (!write_cache_file(path, M)) {
            std::printf("  small case: file write failed (errno=%d)\n", errno);
            return 1;
        }

        WeightedDigraph control = WeightedDigraph::from_dense(M);
        auto loaded = WeightedDigraph::from_dense_file(path);
        EXPECT(loaded.has_value(),
               "small case: from_dense_file returns optional on valid file");
        if (loaded.has_value()) {
            EXPECT(loaded->n == 5,
                   "small case: |V| = 5 matches the written N");
            EXPECT(digraphs_equal(control, *loaded),
                   "small case: streaming-loaded digraph matches in-memory-loaded");
        }
        std::remove(path.c_str());
    }

    // ---- (2) random sparse matrix, N=100 with ~5% density ----
    {
        std::mt19937 rng(12345);
        std::size_t N = 100;
        double density = 0.05;
        std::vector<std::vector<long long>> M(N,
                                                std::vector<long long>(N, 0));
        std::uniform_real_distribution<double> uni(0.0, 1.0);
        for (std::size_t i = 0; i < N; ++i) {
            for (std::size_t j = 0; j < N; ++j) {
                if (uni(rng) < density) {
                    std::uniform_int_distribution<long long> W(1, 9);
                    M[i][j] = W(rng);
                }
            }
        }
        std::string path = "/tmp/wd_roundtrip_random.gbm";
        if (!write_cache_file(path, M)) {
            std::printf("  random case: file write failed\n");
            return 1;
        }

        WeightedDigraph control = WeightedDigraph::from_dense(M);
        auto loaded = WeightedDigraph::from_dense_file(path);
        EXPECT(loaded.has_value(),
               "random case: from_dense_file returns optional");
        if (loaded.has_value()) {
            EXPECT(loaded->n == N,
                   "random case: |V| = 100 matches written N");
            EXPECT(digraphs_equal(control, *loaded),
                   "random case: streaming-loaded digraph matches");
        }
        std::remove(path.c_str());
    }

    // ---- (3) corruption: bad magic should yield nullopt ----
    {
        std::string path = "/tmp/wd_roundtrip_corrupt.gbm";
        int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        const char bad_magic[8] = {'X','X','X','X','X','X','X','X'};
        EXPECT(fd >= 0 && ::write(fd, bad_magic, 8) == 8,
               "corruption case: fixture write succeeds");
        if (fd >= 0) ::close(fd);
        auto loaded = WeightedDigraph::from_dense_file(path);
        EXPECT(!loaded.has_value(),
               "corruption case: bad magic returns nullopt (not a partial digraph)");
        std::remove(path.c_str());
    }

    // ---- (4) corruption: footer mismatch should yield nullopt ----
    {
        std::string path = "/tmp/wd_roundtrip_corrupt_footer.gbm";
        std::vector<std::vector<long long>> M = {
            {0, 1, 0},
            {0, 0, 1},
            {1, 0, 0}};
        if (!write_cache_file(path, M)) {
            std::printf("  corrupt-footer case: file write failed\n");
            return 1;
        }
        // Flip a single byte near the end of the file -- within the
        // data section so magic stays intact, but the FNV-1a
        // checksum in the footer will now mismatch on read.
        int fd = ::open(path.c_str(), O_RDWR);
        struct stat st;
        bool corrupted = fd >= 0 && ::fstat(fd, &st) == 0;
        if (corrupted) {
            off_t mid = static_cast<off_t>(st.st_size) / 2;
            corrupted = ::pwrite(fd, "Z", 1, mid) == 1;
        }
        EXPECT(corrupted, "corrupt-footer case: fixture mutation succeeds");
        if (fd >= 0) ::close(fd);
        auto loaded = WeightedDigraph::from_dense_file(path);
        EXPECT(!loaded.has_value(),
               "corrupt-footer case: footer mismatch returns nullopt");
        std::remove(path.c_str());
    }

    // ---- (5) missing file: should yield nullopt (not crash) ----
    {
        auto loaded = WeightedDigraph::from_dense_file(
            "/tmp/wd_roundtrip_does_not_exist_xyz_42.gbm");
        EXPECT(!loaded.has_value(),
               "missing-file case: nonexistent path returns nullopt");
    }

    std::printf("\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
