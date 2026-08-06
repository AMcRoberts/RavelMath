#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <filesystem>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ravel/proof/block_height_shell_rank.hpp"
#include "ravel/proof/sign_symmetric_chamber_rank.hpp"

using namespace ravel::proof;

namespace {
constexpr std::size_t O = 11;
constexpr std::size_t F = 1 + 4 * 3 + 3 * 9 + 9 + 2 * 27;
constexpr std::size_t V = O * F;

struct Term { std::uint16_t i; std::int64_t v; };
using Sparse = std::vector<Term>;
struct DeltaTerm { std::uint16_t i; std::int64_t v; };
using Delta = std::vector<DeltaTerm>;
struct Range { std::size_t n; std::int64_t lo, hi; };
struct Witness { ShellState source, target; std::uint8_t n; };

int sign_index(std::int64_t x) { return x < 0 ? 0 : (x == 0 ? 1 : 2); }
std::string chamber(const ShellState& x) {
  std::string r; r.reserve(x.size());
  for (auto v : x) r += v < 0 ? '-' : (v > 0 ? '+' : '0');
  return r;
}
ShellState negate(ShellState x) { for (auto& v : x) v = -v; return x; }
ShellState canonical(const ShellState& x) {
  return negate_sign_chamber(chamber(x)) < chamber(x) ? negate(x) : x;
}

std::array<std::int64_t, O> observables(const ShellState& raw) {
  auto x = canonical(raw);
  auto f = derive_block_height_features(x);
  std::int64_t l = 0, sum = 0, mx = x[0], mn = x[0];
  for (auto v : x) { l += std::llabs(v); sum += v; mx = std::max(mx, v); mn = std::min(mn, v); }
  return {l, f.forcing_support, f.forcing_zero_count, f.forcing_gcd,
          f.moment_variation, f.moment_energy, f.adjacent_energy,
          sum, mx, mn, 1};
}

std::array<std::size_t, 4> roles(std::size_t n) { return {0, 1, n - 2, n - 1}; }
std::vector<std::size_t> active_features(const ShellState& raw) {
  auto x = canonical(raw);
  const auto r = roles(x.size());
  std::vector<std::size_t> a{0};
  std::size_t off = 1;
  for (std::size_t k = 0; k < 4; ++k) a.push_back(off + k * 3 + sign_index(x[r[k]]));
  off += 12;
  const std::array<std::pair<std::size_t, std::size_t>, 3> pairs = {{{r[0],r[1]}, {r[2],r[3]}, {r[1],r[2]}}};
  for (std::size_t k = 0; k < 3; ++k)
    a.push_back(off + k * 9 + 3 * sign_index(x[pairs[k].first]) + sign_index(x[pairs[k].second]));
  off += 27;
  a.push_back(off + 3 * sign_index(x[r[3]]) + sign_index(x[r[0]]));
  off += 9;
  a.push_back(off + 9 * sign_index(x[r[0]]) + 3 * sign_index(x[r[1]]) + sign_index(x[r[2]]));
  off += 27;
  a.push_back(off + 9 * sign_index(x[r[1]]) + 3 * sign_index(x[r[2]]) + sign_index(x[r[3]]));
  return a;
}

Sparse design(const ShellState& x) {
  const auto o = observables(x);
  const auto fs = active_features(x);
  Sparse p;
  p.reserve(fs.size() * O);
  for (auto f : fs) for (std::size_t j = 0; j < O; ++j)
    if (o[j]) p.push_back({static_cast<std::uint16_t>(f * O + j), o[j]});
  std::sort(p.begin(), p.end(), [](auto a, auto b){ return a.i < b.i; });
  return p;
}

Delta difference(const Sparse& s, const Sparse& t) {
  Delta d; d.reserve(s.size() + t.size());
  std::size_t i = 0, j = 0;
  while (i < s.size() || j < t.size()) {
    if (j == t.size() || (i < s.size() && s[i].i < t[j].i)) {
      d.push_back({s[i].i, -s[i].v}); ++i;
    } else if (i == s.size() || t[j].i < s[i].i) {
      d.push_back({t[j].i, t[j].v}); ++j;
    } else {
      const auto v = t[j].v - s[i].v;
      if (v) d.push_back({s[i].i, v});
      ++i; ++j;
    }
  }
  return d;
}

struct DeltaHash {
  std::size_t operator()(const Delta& d) const noexcept {
    std::size_t h = 1469598103934665603ULL;
    for (const auto& q : d) {
      h ^= static_cast<std::size_t>(q.i) * 0x9e3779b97f4a7c15ULL;
      h *= 1099511628211ULL;
      h ^= std::hash<std::int64_t>{}(q.v);
      h *= 1099511628211ULL;
    }
    return h;
  }
};
struct DeltaEq {
  bool operator()(const Delta& a, const Delta& b) const noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) if (a[i].i != b[i].i || a[i].v != b[i].v) return false;
    return true;
  }
};

long double dot(const Delta& d, const std::vector<long double>& w) {
  long double z = 0; for (const auto& q : d) z += w[q.i] * q.v; return z;
}
long double norm2(const Delta& d) {
  long double z = 0; for (const auto& q : d) z += static_cast<long double>(q.v) * q.v; return z;
}
void update(std::vector<long double>& w, const Delta& d, long double k) {
  for (const auto& q : d) w[q.i] += k * q.v;
}

void stream_edges(const Range& rg, const std::function<void(const ShellState&, const ShellState&)>& emit) {
  for (std::int64_t bound = rg.lo; bound <= rg.hi; ++bound) {
    std::vector<ShellState> shell;
    std::unordered_set<ShellState, ShellStateHash> shell_set;
    ShellState zero(rg.n, 0);
    enumerate_box_states_rec(rg.n, bound, 0, zero, [&](const ShellState& x) {
      if (shell_radius(x) == bound) { shell.push_back(x); shell_set.insert(x); }
    });
    for (const auto& source : shell) {
      std::deque<ShellState> queue;
      std::unordered_set<ShellState, ShellStateHash> seen;
      queue.push_back(source); seen.insert(source);
      while (!queue.empty()) {
        auto current = queue.front(); queue.pop_front();
        for (auto target : bounded_carry_successors(current, bound)) {
          if (shell_set.contains(target)) emit(source, target);
          else if (seen.insert(target).second) queue.push_back(std::move(target));
        }
      }
    }
  }
}

void print_int128(__int128 z) {
  if (z < 0) { std::cout << '-'; z = -z; }
  std::string s; if (!z) s = "0";
  while (z) { s.push_back(static_cast<char>('0' + z % 10)); z /= 10; }
  std::reverse(s.begin(), s.end()); std::cout << s;
}
} // namespace

int main(int argc, char** argv) {
  const bool enumerate_only = argc > 1 && std::string(argv[1]) == "--enumerate-only";
  const std::vector<Range> ranges = {{3,2,8}, {4,2,6}, {5,2,4}};
  std::unordered_map<ShellState, Sparse, ShellStateHash> design_cache;
  auto get_design = [&](const ShellState& x) -> const Sparse& {
    auto [it, fresh] = design_cache.try_emplace(x);
    if (fresh) it->second = design(x);
    return it->second;
  };

  std::unordered_map<Delta, Witness, DeltaHash, DeltaEq> types;
  std::map<std::size_t, std::uint64_t> concrete_by_n;
  for (const auto& rg : ranges) {
    stream_edges(rg, [&](const ShellState& s, const ShellState& t) {
      ++concrete_by_n[rg.n];
      auto d = difference(get_design(s), get_design(t));
      types.try_emplace(std::move(d), Witness{s,t,static_cast<std::uint8_t>(rg.n)});
    });
    std::cerr << "compressed n=" << rg.n << " concrete=" << concrete_by_n[rg.n]
              << " delta_types=" << types.size() << " cached_states=" << design_cache.size() << "\n";
  }

  std::vector<const Delta*> constraints; constraints.reserve(types.size());
  for (const auto& [d,w] : types) constraints.push_back(&d);
  std::filesystem::create_directories("out");
  {
    std::ofstream binary("out/lazy_sparse_boundary_deltas.bin", std::ios::binary);
    const std::uint64_t count = constraints.size();
    binary.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto* d : constraints) {
      const std::uint32_t size = d->size();
      binary.write(reinterpret_cast<const char*>(&size), sizeof(size));
      binary.write(reinterpret_cast<const char*>(d->data()), sizeof(DeltaTerm) * d->size());
    }
  }
  if (enumerate_only) {
    std::uint64_t concrete = 0; for (auto [n,c] : concrete_by_n) concrete += c;
    std::cout << "LAZY_SPARSE_ENUMERATION concrete_edges=" << concrete
              << " delta_types=" << constraints.size()
              << " cached_states=" << design_cache.size()
              << " variables=" << V << "\n";
    return 0;
  }
  std::vector<long double> weights(V, 0.0L);
  {
    std::ifstream warm("out/boundary_flux_weights.tsv");
    std::string header; std::getline(warm, header);
    std::size_t f=0,j=0; long double value=0;
    while (warm >> f >> j >> value) if (f*O+j < V) weights[f*O+j] = value / 1000000.0L;
  }
  struct Violation { const Delta* d; long double margin; };
  constexpr std::size_t batch_cap = 4096;
  for (int round = 0; round < 120; ++round) {
    std::vector<Violation> bad; bad.reserve(batch_cap * 2);
    long double worst = std::numeric_limits<long double>::infinity();
    std::size_t total_bad = 0;
    for (const auto* d : constraints) {
      const auto margin = dot(*d, weights); worst = std::min(worst, margin);
      if (margin < 1) { ++total_bad; bad.push_back({d, margin}); }
    }
    if (bad.size() > batch_cap) {
      std::nth_element(bad.begin(), bad.begin()+batch_cap, bad.end(),
        [](const Violation&a,const Violation&b){return a.margin<b.margin;});
      bad.resize(batch_cap);
    }
    std::cerr << "round=" << round << " violations=" << total_bad << " batch=" << bad.size()
              << " worst=" << static_cast<double>(worst) << "\n";
    if (!total_bad) break;
    for (int inner = 0; inner < 400; ++inner) {
      std::size_t updates = 0;
      for (const auto& v : bad) {
        const auto margin = dot(*v.d, weights);
        if (margin < 1) { const auto q = norm2(*v.d); if (q) { update(weights, *v.d, (1-margin)/q); ++updates; } }
      }
      if (!updates) break;
    }
  }

  std::vector<std::int64_t> integer_weights(V);
  for (std::size_t i = 0; i < V; ++i) integer_weights[i] = std::llround(weights[i] * 1000000.L);
  auto integer_margin = [&](const Delta& d) {
    __int128 z = 0; for (const auto& q : d) z += static_cast<__int128>(integer_weights[q.i]) * q.v; return z;
  };

  std::size_t type_failures = 0; __int128 type_min = static_cast<__int128>(1) << 120;
  for (const auto* d : constraints) { auto m = integer_margin(*d); type_min = std::min(type_min,m); if (m <= 0) ++type_failures; }

  // Final proof replay regenerates the graph and stores nothing except the smallest obstruction.
  std::uint64_t replay_edges = 0, replay_failures = 0; __int128 replay_min = static_cast<__int128>(1) << 120;
  Witness worst_witness; bool have_worst = false;
  for (const auto& rg : ranges) {
    stream_edges(rg, [&](const ShellState& s, const ShellState& t) {
      ++replay_edges;
      const auto d = difference(get_design(s), get_design(t));
      const auto m = integer_margin(d);
      if (!have_worst || m < replay_min) { replay_min = m; worst_witness = {s,t,static_cast<std::uint8_t>(rg.n)}; have_worst = true; }
      if (m <= 0) ++replay_failures;
    });
  }

  std::filesystem::create_directories("out");
  std::ofstream wf("out/lazy_sparse_boundary_rank_weights.tsv");
  wf << "index\tweight\n";
  for (std::size_t i = 0; i < V; ++i) if (integer_weights[i]) wf << i << '\t' << integer_weights[i] << '\n';

  std::cout << "LAZY_SPARSE_BOUNDARY_RANK"
            << " concrete_edges=" << replay_edges
            << " delta_types=" << constraints.size()
            << " cached_states=" << design_cache.size()
            << " variables=" << V
            << " type_failures=" << type_failures
            << " replay_failures=" << replay_failures
            << " exact_minimum_gain=";
  print_int128(replay_min);
  std::cout << "\n";
  if (replay_failures && have_worst) {
    std::cout << "WORST n=" << static_cast<int>(worst_witness.n) << " source=";
    for (auto v : worst_witness.source) std::cout << v << ',';
    std::cout << " target="; for (auto v : worst_witness.target) std::cout << v << ',';
    std::cout << "\n";
  }
  return type_failures || replay_failures ? 3 : 0;
}
