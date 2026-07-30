// lua_bindings.cpp
//
// Author: Ravel
// Coordinator and project architect: AM
//
// Lua C-API bindings for the ravel backend library.  Exposes:
//   ravel.cyclotomic.*      -- 12th cyclotomic ring arithmetic
//   ravel.constants.*       -- named decimal constants
//   ravel.spectral.*        -- spectral invariants for Pisot matrices,
//                                  plus the general algebraic path:
//                                  charpoly_int(M), qbeta_eigenvalue(M, c)
//                                  for arbitrary-d exact arithmetic
//                                  (replaces the 8-fixed-ring legacy
//                                  path in algebraic.hpp)
//   ravel.tilt.*            -- Hall-Branciard tilt model
//   ravel.substitution.*    -- primitive Pisot substitution machinery
//   ravel.balanced_pair.*   -- Hollander-Solomyak certification + rho_nc
//   ravel.thermometer.*     -- Pisot spectral-type thermometer
//   ravel.barge.*           -- Barge flags + Pisot classifier
//   ravel.contact_boundary.* -- contact-boundary-map pipeline (W10)
//                                  including both single-substitution
//                                  `compute` and `batch_run` for families
//
// The Cyclotomic12D ring is exchanged with Lua as a 4-integer array
// {a, b, c, e}.  Substitutions are exchanged as { word : {letter : int}
// } maps, with alphabet size inferred from the largest letter index.

#include <lua5.4/lua.hpp>

#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ravel/cyclotomic.hpp"
#include "ravel/spectral.hpp"
#include "ravel/qbeta_eigenvalue.hpp"
#include "ravel/tilt.hpp"
#include "ravel/substitution.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/bp_dump_analysis.hpp"
#include "ravel/fibonacci_selection.hpp"
#include "ravel/fibonacci_finite.hpp"
#include "ravel/packed_binary_dynamics.hpp"
#include "ravel/thermometer.hpp"
#include "ravel/barge.hpp"
#include "ravel/survey.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/rauzy_fractal.hpp"
#include "ravel/return_substitution.hpp"
#include "ravel/contact_boundary.hpp"

namespace {

using ravel::Cyclotomic12D;
using ravel::SpectralInvariants;
using ravel::SubstitutionRule;
template <std::size_t d> using DCandidate = ::ravel::DCandidate<d>;
template <std::size_t d> using Substitution = ravel::Substitution<d>;
using ravel::BalancedPairCertificate;
using ravel::ThermometerResult;
using ravel::BargeFlags;
using ravel::PisotClassify;
using ravel::barge_flags;

Cyclotomic12D lua_check_cyclotomic(lua_State* L, int idx) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_rawgeti(L, idx, 1);
    lua_rawgeti(L, idx, 2);
    lua_rawgeti(L, idx, 3);
    lua_rawgeti(L, idx, 4);
    std::int64_t a = static_cast<std::int64_t>(lua_tointeger(L, -4));
    std::int64_t b = static_cast<std::int64_t>(lua_tointeger(L, -3));
    std::int64_t c = static_cast<std::int64_t>(lua_tointeger(L, -2));
    std::int64_t e = static_cast<std::int64_t>(lua_tointeger(L, -1));
    lua_pop(L, 4);
    return {a, b, c, e};
}

void lua_push_cyclotomic(lua_State* L, const Cyclotomic12D& z) {
    lua_createtable(L, 4, 0);
    lua_pushinteger(L, static_cast<lua_Integer>(z.a)); lua_rawseti(L, -2, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(z.b)); lua_rawseti(L, -2, 2);
    lua_pushinteger(L, static_cast<lua_Integer>(z.c)); lua_rawseti(L, -2, 3);
    lua_pushinteger(L, static_cast<lua_Integer>(z.e)); lua_rawseti(L, -2, 4);
}

// ---------- cyclotomic ----------

int l_cyclotomic_zero(lua_State* L) {
    lua_push_cyclotomic(L, Cyclotomic12D::zero());
    return 1;
}
int l_cyclotomic_one(lua_State* L) {
    lua_push_cyclotomic(L, Cyclotomic12D::one());
    return 1;
}
int l_cyclotomic_d(lua_State* L) {
    lua_push_cyclotomic(L, Cyclotomic12D::d());
    return 1;
}
int l_cyclotomic_d2(lua_State* L) {
    lua_push_cyclotomic(L, Cyclotomic12D::d2());
    return 1;
}
int l_cyclotomic_d3(lua_State* L) {
    lua_push_cyclotomic(L, Cyclotomic12D::d3());
    return 1;
}

int l_cyclotomic_make(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    luaL_checktype(L, 4, LUA_TNUMBER);
    Cyclotomic12D z{
        static_cast<std::int64_t>(lua_tointeger(L, 1)),
        static_cast<std::int64_t>(lua_tointeger(L, 2)),
        static_cast<std::int64_t>(lua_tointeger(L, 3)),
        static_cast<std::int64_t>(lua_tointeger(L, 4))
    };
    lua_push_cyclotomic(L, z);
    return 1;
}

int l_cyclotomic_add(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    auto y = lua_check_cyclotomic(L, 2);
    lua_push_cyclotomic(L, x + y);
    return 1;
}
int l_cyclotomic_sub(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    auto y = lua_check_cyclotomic(L, 2);
    lua_push_cyclotomic(L, x - y);
    return 1;
}
int l_cyclotomic_mul(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    auto y = lua_check_cyclotomic(L, 2);
    lua_push_cyclotomic(L, x * y);
    return 1;
}
int l_cyclotomic_neg(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    lua_push_cyclotomic(L, -x);
    return 1;
}
int l_cyclotomic_scale(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    luaL_checktype(L, 2, LUA_TNUMBER);
    std::int64_t k = static_cast<std::int64_t>(lua_tointeger(L, 2));
    lua_push_cyclotomic(L, x * k);
    return 1;
}
int l_cyclotomic_eq(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    auto y = lua_check_cyclotomic(L, 2);
    lua_pushboolean(L, x == y ? 1 : 0);
    return 1;
}
int l_cyclotomic_tostring(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    char buf[128];
    std::snprintf(buf, sizeof(buf),
                  "(%lld + %lld*d + %lld*d^2 + %lld*d^3)",
                  static_cast<long long>(x.a), static_cast<long long>(x.b),
                  static_cast<long long>(x.c), static_cast<long long>(x.e));
    lua_pushstring(L, buf);
    return 1;
}
int l_cyclotomic_to_xy(lua_State* L) {
    auto x = lua_check_cyclotomic(L, 1);
    auto xy = ravel::cyclotomic_to_xy(x);
    lua_createtable(L, 2, 0);
    lua_pushnumber(L, xy[0]); lua_rawseti(L, -2, 1);
    lua_pushnumber(L, xy[1]); lua_rawseti(L, -2, 2);
    return 1;
}

// ---------- constants ----------

int l_const_half(lua_State* L)       { lua_pushnumber(L, static_cast<double>(ravel::kHalf));         return 1; }
int l_const_one(lua_State* L)        { lua_pushnumber(L, static_cast<double>(ravel::kOne));          return 1; }
int l_const_zero(lua_State* L)       { lua_pushnumber(L, static_cast<double>(ravel::kZero));         return 1; }
int l_const_sqrt3over2(lua_State* L) { lua_pushnumber(L, static_cast<double>(ravel::kSqrt3Over2));   return 1; }
int l_const_dx(lua_State* L)         { lua_pushnumber(L, static_cast<double>(ravel::kDGeneratorX));  return 1; }
int l_const_dy(lua_State* L)         { lua_pushnumber(L, static_cast<double>(ravel::kDGeneratorY));  return 1; }
int l_const_d2x(lua_State* L)        { lua_pushnumber(L, static_cast<double>(ravel::kD2GeneratorX)); return 1; }
int l_const_d2y(lua_State* L)        { lua_pushnumber(L, static_cast<double>(ravel::kD2GeneratorY)); return 1; }
int l_const_d3x(lua_State* L)        { lua_pushnumber(L, static_cast<double>(ravel::kD3GeneratorX)); return 1; }
int l_const_d3y(lua_State* L)        { lua_pushnumber(L, static_cast<double>(ravel::kD3GeneratorY)); return 1; }

// ---------- spectral ----------

void lua_push_invariants(lua_State* L, const SpectralInvariants& inv) {
    lua_createtable(L, 0, 9);
    lua_pushinteger(L, static_cast<lua_Integer>(inv.n));        lua_setfield(L, -2, "n");
    lua_pushnumber  (L, inv.beta);                              lua_setfield(L, -2, "beta");
    lua_pushnumber  (L, inv.beta_abs);                         lua_setfield(L, -2, "beta_abs");
    lua_pushnumber  (L, inv.beta2);                             lua_setfield(L, -2, "beta2");
    lua_pushinteger (L, static_cast<lua_Integer>(inv.det_M));   lua_setfield(L, -2, "det_M");
    lua_pushinteger (L, static_cast<lua_Integer>(inv.abs_det)); lua_setfield(L, -2, "abs_det");
    lua_pushboolean (L, inv.invol_safe ? 1 : 0);                lua_setfield(L, -2, "invol_safe");
    lua_pushnumber  (L, inv.bound_rhs);                        lua_setfield(L, -2, "bound_rhs");
    lua_pushboolean (L, inv.bound_holds ? 1 : 0);               lua_setfield(L, -2, "bound_holds");
}

int l_spectral_2x2(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    luaL_checktype(L, 4, LUA_TNUMBER);
    auto inv = ravel::spectral_invariants_2x2(
        static_cast<long long>(lua_tointeger(L, 1)),
        static_cast<long long>(lua_tointeger(L, 2)),
        static_cast<long long>(lua_tointeger(L, 3)),
        static_cast<long long>(lua_tointeger(L, 4)));
    lua_push_invariants(L, inv);
    return 1;
}

int l_spectral_3x3(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    long long m[3][3];
    for (int i = 0; i < 3; ++i) {
        lua_rawgeti(L, 1, i + 1);
        if (!lua_istable(L, -1)) {
            return luaL_error(L, "spectral_3x3: expected 3x3 matrix table");
        }
        for (int j = 0; j < 3; ++j) {
            lua_rawgeti(L, -1, j + 1);
            m[i][j] = static_cast<long long>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    auto inv = ravel::spectral_invariants_3x3(
        m[0][0], m[0][1], m[0][2],
        m[1][0], m[1][1], m[1][2],
        m[2][0], m[2][1], m[2][2]);
    lua_push_invariants(L, inv);
    return 1;
}

// ---------- spectral: general algebraic path (arbitrary d) ----------
//
// The 8-fixed-ring legacy path in algebraic.hpp covers Fibonacci / Sqrt2 /
// Tribonacci / Plastic / Supergolden / sigma_1 / sigma_2 / Tetrabonacci
// only, which is what the 11-candidate follow-up survey used.  The general
// path on the C++ side is:
//   * barge.hpp::charpoly_int(M)        -- exact integer charpoly of M
//   * qbeta_eigenvalue.hpp::qbeta_dominant_eigenvalue(M, c)
//                                       -- exact Q(β) dominant eigenvalue
//                                          for the matrix M with charpoly c
//
// These three bindings expose the general path to Lua so the broad 39-substitution
// and 87-candidate surveys can be run with bit-exact arithmetic at every
// matrix size, not just the hand-picked 11.  See docs/DIRECTION_AND_OPEN_THREADS.md
// thread C item 1 and docs/RESEARCH_VECTORS.md #3.

// charpoly_int(M) -> { c0, c1, ..., cd } (lowest-degree first, monic)
//   Computes the exact integer charpoly of M via Newton's identities on traces.
//   No 8-ring hardcoding; works for any d >= 1.
int l_spectral_charpoly_int(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    const std::size_t n = static_cast<std::size_t>(lua_rawlen(L, 1));
    std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<int>(i) + 1);
        if (!lua_istable(L, -1)) {
            return luaL_error(L, "charpoly_int: row %zu is not a table", i);
        }
        for (std::size_t j = 0; j < n; ++j) {
            lua_rawgeti(L, -1, static_cast<int>(j) + 1);
            M[i][j] = static_cast<long long>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    auto cp = ravel::charpoly_int(M);
    lua_createtable(L, static_cast<int>(cp.size()), 0);
    for (std::size_t i = 0; i < cp.size(); ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(cp[i]));
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

// qbeta_eigenvalue(M, c, [k_max]) -> { lambda, converged, steps_taken,
//   peak_memory_bytes, error }
//   Computes the dominant eigenvalue of M using exact Q(β) arithmetic
//   where β is the unique real root > 1 of the monic charpoly
//   x^d + c[0] x^(d-1) + ... + c[d-1] = 0 (c is the vector from
//   charpoly_int(M), or any other Pisot charpoly).  Bit-exact for any d.
int l_spectral_qbeta_eigenvalue(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);  // M
    luaL_checktype(L, 2, LUA_TTABLE);  // c
    // Matrix
    const std::size_t n = static_cast<std::size_t>(lua_rawlen(L, 1));
    std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<int>(i) + 1);
        for (std::size_t j = 0; j < n; ++j) {
            lua_rawgeti(L, -1, static_cast<int>(j) + 1);
            M[i][j] = static_cast<long long>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    // Charpoly: LUA table is barge.hpp::charpoly_int's output, with
    // the leading coefficient at index 1 (LUA 1-indexed) and the
    // constant at index n+1.  Length is n+1 for degree n.  The qbeta
    // C++ layer expects c.size() == n with c[0] = coefficient of
    // x^{n-1} (skipping the leading 1).
    const std::size_t deg = static_cast<std::size_t>(lua_rawlen(L, 2)) - 1;
    if (deg == 0 || deg > 64) {
        return luaL_error(L, "qbeta_eigenvalue: charpoly length must be in [2, 65] (got %zu)",
                          static_cast<std::size_t>(lua_rawlen(L, 2)));
    }
    ravel::QBetaCharPoly poly;
    poly.c.assign(deg, 0);
    for (std::size_t i = 0; i < deg; ++i) {
        lua_rawgeti(L, 2, static_cast<int>(i) + 2);  // skip LUA[1] (the leading 1)
        poly.c[i] = static_cast<long long>(lua_tointeger(L, -1));
        lua_pop(L, 1);
    }
    // k_max default: 80 (matches the standalone CLI).
    int k_max = 80;
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        k_max = static_cast<int>(lua_tointeger(L, 3));
    }
    ravel::QBetaOptions opts;
    opts.k_max = k_max;
    auto result = ravel::qbeta_dominant_eigenvalue(M, poly, opts);
    // Return: {lambda, converged, steps_taken, peak_memory_bytes, error}
    lua_createtable(L, 0, 5);
    lua_pushnumber(L, result.lambda);  lua_setfield(L, -2, "lambda");
    lua_pushboolean(L, result.converged ? 1 : 0);  lua_setfield(L, -2, "converged");
    lua_pushinteger(L, static_cast<lua_Integer>(result.steps_taken));
        lua_setfield(L, -2, "steps_taken");
    lua_pushinteger(L, static_cast<lua_Integer>(result.peak_memory_bytes));
        lua_setfield(L, -2, "peak_memory_bytes");
    lua_pushstring(L, result.error.c_str());  lua_setfield(L, -2, "error");
    return 1;
}

// ---------- tilt ----------

int l_tilt_tsirelson(lua_State* L) { lua_pushnumber(L, ravel::tsirelson_bound()); return 1; }
int l_tilt_kl2(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);
    lua_pushnumber(L, ravel::kl2_bits(lua_tonumber(L, 1), lua_tonumber(L, 2)));
    return 1;
}
int l_tilt_hb_floor(lua_State* L) { lua_pushnumber(L, ravel::hb_mi_floor()); return 1; }
int l_tilt_correlator(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);
    auto i = static_cast<std::size_t>(lua_tointeger(L, 1));
    auto j = static_cast<std::size_t>(lua_tointeger(L, 2));
    if (i >= ravel::kTiltSettings || j >= ravel::kTiltSettings) {
        return luaL_error(L, "tilt correlator index out of range");
    }
    lua_pushnumber(L, ravel::tilt_correlator(i, j));
    return 1;
}
int l_tilt_silver_cost(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    lua_pushnumber(L, ravel::tilt_silver_cost_per_angle(lua_tonumber(L, 1)));
    return 1;
}

// ---------- substitution ----------

SubstitutionRule lua_check_substitution(lua_State* L, int idx) {
    luaL_checktype(L, idx, LUA_TTABLE);
    const std::size_t n = static_cast<std::size_t>(lua_rawlen(L, idx));
    std::vector<std::vector<std::int8_t>> sigma;
    for (std::size_t c = 0; c < n; ++c) {
        lua_rawgeti(L, idx, static_cast<int>(c) + 1);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            luaL_error(L, "substitution: column %d is not a list of ints", c + 1);
        }
        std::vector<std::int8_t> word;
        const std::size_t wlen = static_cast<std::size_t>(lua_rawlen(L, -1));
        word.reserve(wlen);
        for (std::size_t i = 0; i < wlen; ++i) {
            lua_rawgeti(L, -1, static_cast<int>(i) + 1);
            word.push_back(static_cast<std::int8_t>(lua_tointeger(L, -1)));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        sigma.push_back(std::move(word));
    }
    return SubstitutionRule(std::move(sigma));
}

int l_subst_new(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(r.alphabet_size()));
    return 1;
}

int l_subst_apply_once(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    std::vector<std::int8_t> word;
    const std::size_t n = static_cast<std::size_t>(lua_rawlen(L, 2));
    word.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 2, static_cast<int>(i) + 1);
        word.push_back(static_cast<std::int8_t>(lua_tointeger(L, -1)));
        lua_pop(L, 1);
    }
    auto out = r.apply_once(word);
    lua_createtable(L, static_cast<int>(out.size()), 0);
    for (std::size_t i = 0; i < out.size(); ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(out[i]));
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

int l_subst_expand(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    auto out = r.expand(
        static_cast<std::int8_t>(lua_tointeger(L, 2)),
        static_cast<std::size_t>(lua_tointeger(L, 3)));
    lua_createtable(L, static_cast<int>(out.size()), 0);
    for (std::size_t i = 0; i < out.size(); ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(out[i]));
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

int l_subst_matrix(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    auto M = r.matrix();
    const std::size_t n = M.size();
    lua_createtable(L, static_cast<int>(n), 0);
    for (std::size_t i = 0; i < n; ++i) {
        lua_createtable(L, static_cast<int>(n), 0);
        for (std::size_t j = 0; j < n; ++j) {
            lua_pushinteger(L, static_cast<lua_Integer>(M[i][j]));
            lua_rawseti(L, -2, static_cast<int>(j) + 1);
        }
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

int l_subst_alpha_size(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(r.alphabet_size()));
    return 1;
}

int l_return_phase_build(lua_State* L) {
    auto rule = lua_check_substitution(L, 1);
    lua_Integer marker_value = luaL_optinteger(L, 2, 0);
    lua_Integer cap_value = luaL_optinteger(L, 3, 1 << 20);
    if (marker_value < 0
        || static_cast<std::size_t>(marker_value) >= rule.alphabet_size()) {
        return luaL_error(L, "return_phase.build: marker out of range");
    }
    if (cap_value < 64) {
        return luaL_error(L, "return_phase.build: orbit cap must be >= 64");
    }
    ravel::ReturnPhaseSystem system;
    try {
        system = ravel::build_return_phase_system(
            rule, static_cast<std::int8_t>(marker_value),
            static_cast<std::size_t>(cap_value));
    } catch (const std::exception& error) {
        return luaL_error(L, "return_phase.build: %s", error.what());
    }

    lua_createtable(L, 0, 5);
    lua_pushinteger(L, marker_value);
    lua_setfield(L, -2, "marker");

    lua_createtable(
        L, static_cast<int>(system.induced.words.size()), 0);
    for (std::size_t r = 0; r < system.induced.words.size(); ++r) {
        lua_createtable(
            L, static_cast<int>(system.induced.words[r].size()), 0);
        for (std::size_t i = 0; i < system.induced.words[r].size(); ++i) {
            lua_pushinteger(L, system.induced.words[r][i]);
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
        lua_rawseti(L, -2, static_cast<int>(r) + 1);
    }
    lua_setfield(L, -2, "return_words");

    lua_createtable(
        L, static_cast<int>(system.induced.derived_images.size()), 0);
    for (std::size_t r = 0;
         r < system.induced.derived_images.size(); ++r) {
        lua_createtable(
            L, static_cast<int>(system.induced.derived_images[r].size()), 0);
        for (std::size_t i = 0;
             i < system.induced.derived_images[r].size(); ++i) {
            lua_pushinteger(L, static_cast<lua_Integer>(
                system.induced.derived_images[r][i]));
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
        lua_rawseti(L, -2, static_cast<int>(r) + 1);
    }
    lua_setfield(L, -2, "derived_images");

    lua_createtable(L, static_cast<int>(system.states.size()), 0);
    for (std::size_t state = 0; state < system.states.size(); ++state) {
        lua_createtable(L, 0, 3);
        lua_pushinteger(L, static_cast<lua_Integer>(
            system.states[state].return_word));
        lua_setfield(L, -2, "return_word");
        lua_pushinteger(L, static_cast<lua_Integer>(
            system.states[state].offset));
        lua_setfield(L, -2, "offset");
        lua_createtable(
            L, static_cast<int>(system.phase_images[state].size()), 0);
        for (std::size_t i = 0;
             i < system.phase_images[state].size(); ++i) {
            lua_pushinteger(L, static_cast<lua_Integer>(
                system.phase_images[state][i]));
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
        lua_setfield(L, -2, "image");
        lua_rawseti(L, -2, static_cast<int>(state) + 1);
    }
    lua_setfield(L, -2, "states");

    auto charpoly = ravel::charpoly_int(system.incidence_matrix());
    lua_createtable(L, static_cast<int>(charpoly.size()), 0);
    for (std::size_t i = 0; i < charpoly.size(); ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(charpoly[i]));
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "charpoly_low_first");
    return 1;
}

// ---------- balanced_pair ----------

int l_bp_certify(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    std::size_t max_pairs = 20000;
    std::size_t max_len   = 4000;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        luaL_checktype(L, 2, LUA_TNUMBER);
        max_pairs = static_cast<std::size_t>(lua_tointeger(L, 2));
    }
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        max_len = static_cast<std::size_t>(lua_tointeger(L, 3));
    }
    auto cert = ravel::balanced_pair_certify(r, max_pairs, max_len);
    lua_createtable(L, 0, 5);
    lua_pushboolean(L, cert.certified ? 1 : 0);  lua_setfield(L, -2, "certified");
    lua_pushboolean(L, cert.terminated ? 1 : 0);  lua_setfield(L, -2, "terminated");
    lua_pushinteger(L, static_cast<lua_Integer>(cert.n_irreducible)); lua_setfield(L, -2, "n_irreducible");
    lua_pushinteger(L, static_cast<lua_Integer>(cert.n_coincidences)); lua_setfield(L, -2, "n_coincidences");
    lua_pushinteger(L, static_cast<lua_Integer>(cert.max_image_length)); lua_setfield(L, -2, "max_image_length");
    return 1;
}

int l_bp_rho_nc(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    std::size_t max_pairs = 20000;
    std::size_t max_len   = 60000;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) max_pairs = static_cast<std::size_t>(lua_tointeger(L, 2));
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) max_len   = static_cast<std::size_t>(lua_tointeger(L, 3));
    double rho = ravel::rho_nc(r, max_pairs, max_len);
    lua_pushnumber(L, rho);
    return 1;
}

void push_bp_dump_analysis(lua_State* L,
                           const ravel::BpDumpAnalysis& result) {
    lua_createtable(L, 0, 12);
#define SET_INT(field) do { \
    lua_pushinteger(L, static_cast<lua_Integer>(result.field)); \
    lua_setfield(L, -2, #field); \
} while (0)
#define SET_BOOL(field) do { \
    lua_pushboolean(L, result.field ? 1 : 0); \
    lua_setfield(L, -2, #field); \
} while (0)
    SET_INT(n);
    SET_INT(core_size);
    SET_INT(edge_records);
    SET_INT(extra_rows);
    SET_BOOL(correction_nilpotent);
    SET_INT(nilpotency_index);
    SET_BOOL(correction_lower_triangular_by_length);
    SET_INT(branching_states);
    SET_BOOL(chunk_counts_are_powers_of_two);
    SET_BOOL(position_zero_is_transient);
    SET_BOOL(designated_is_max_v2_position);
#undef SET_BOOL
#undef SET_INT
}

ravel::BpDumpLimits read_bp_dump_limits(lua_State* L, int first) {
    ravel::BpDumpLimits limits;
    if (lua_gettop(L) >= first && !lua_isnil(L, first))
        limits.max_bytes = static_cast<std::size_t>(
            luaL_checkinteger(L, first));
    if (lua_gettop(L) >= first + 1 && !lua_isnil(L, first + 1))
        limits.max_states = static_cast<std::size_t>(
            luaL_checkinteger(L, first + 1));
    if (lua_gettop(L) >= first + 2 && !lua_isnil(L, first + 2))
        limits.max_edges = static_cast<std::size_t>(
            luaL_checkinteger(L, first + 2));
    if (limits.max_bytes == 0 || limits.max_states == 0
        || limits.max_edges == 0) {
        luaL_error(L, "BP dump limits must be positive");
    }
    return limits;
}

int l_bp_analyze_dump(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    auto limits = read_bp_dump_limits(L, 2);
    try {
        auto result = ravel::analyze_bp_dump_file(path, limits);
        push_bp_dump_analysis(L, result);
        return 1;
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

int l_bp_analyze_dump_text(lua_State* L) {
    std::size_t size = 0;
    const char* text = luaL_checklstring(L, 1, &size);
    auto limits = read_bp_dump_limits(L, 2);
    try {
        std::istringstream input(std::string(text, size));
        auto result = ravel::analyze_bp_dump(input, limits);
        push_bp_dump_analysis(L, result);
        return 1;
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

// ---------- thermometer ----------

int l_thermometer(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    std::int8_t seed = 0;
    std::size_t n_seq = 1u << 14;
    std::size_t lmax  = 1u << 12;
    std::size_t lmin  = 33;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) seed = static_cast<std::int8_t>(lua_tointeger(L, 2));
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) n_seq = static_cast<std::size_t>(lua_tointeger(L, 3));
    if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) lmax  = static_cast<std::size_t>(lua_tointeger(L, 4));
    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) lmin  = static_cast<std::size_t>(lua_tointeger(L, 5));
    auto th = ravel::thermometer(r, seed, n_seq, lmax, lmin);
    lua_createtable(L, 0, 2);
    lua_pushnumber(L, th.theta);                          lua_setfield(L, -2, "theta");
    lua_createtable(L, static_cast<int>(th.records.size()), 0);
    for (std::size_t i = 0; i < th.records.size(); ++i) {
        lua_createtable(L, 2, 0);
        lua_pushinteger(L, static_cast<lua_Integer>(th.records[i].first));
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, th.records[i].second);
        lua_rawseti(L, -2, 2);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "records");
    return 1;
}

// ---------- barge ----------

int l_barge_flags(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    std::vector<std::vector<std::int8_t>> sigma(r.alphabet_size());
    for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
        sigma[c] = r.image(c);
    }
    auto f = barge_flags(sigma);
    lua_createtable(L, 0, 2);
    lua_pushboolean(L, f.distinct_initials ? 1 : 0); lua_setfield(L, -2, "distinct_initials");
    lua_pushboolean(L, f.constant_finals   ? 1 : 0); lua_setfield(L, -2, "constant_finals");
    return 1;
}

// ---------- survey ----------

// random_pisot_survey(target, seed, max_pairs, max_len, alphabet_size)
//   -> list of instances.
// Each instance is a Lua table with fields name, beta, b2,
// certified, n_irreducible, rho_nc, pisot (bool).
// alphabet_size: 0 (default, mixed n in {2,3,4}) or fixed n >= 2.
int l_survey_random(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    std::size_t target = static_cast<std::size_t>(lua_tointeger(L, 1));
    std::uint32_t seed = 11;
    std::size_t max_pairs_cert = 8000;
    std::size_t max_len_cert   = 60000;
    int alphabet_size = 0;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        luaL_checktype(L, 2, LUA_TNUMBER);
        seed = static_cast<std::uint32_t>(lua_tointeger(L, 2));
    }
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        max_pairs_cert = static_cast<std::size_t>(lua_tointeger(L, 3));
    }
    if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
        luaL_checktype(L, 4, LUA_TNUMBER);
        max_len_cert = static_cast<std::size_t>(lua_tointeger(L, 4));
    }
    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
        luaL_checktype(L, 5, LUA_TNUMBER);
        alphabet_size = static_cast<int>(lua_tointeger(L, 5));
    }
    auto instances = ravel::random_pisot_survey(
        target, seed, max_pairs_cert, max_len_cert, max_pairs_cert, max_len_cert,
        /*max_trials=*/8000, alphabet_size);
    lua_createtable(L, static_cast<int>(instances.size()), 0);
    for (std::size_t i = 0; i < instances.size(); ++i) {
        const auto& it = instances[i];
        lua_createtable(L, 0, 9);
        lua_pushstring(L, it.name.c_str());                  lua_setfield(L, -2, "name");
        lua_pushnumber(L, it.beta);                          lua_setfield(L, -2, "beta");
        lua_pushnumber(L, it.b2);                            lua_setfield(L, -2, "b2");
        lua_pushboolean(L, it.certified ? 1 : 0);            lua_setfield(L, -2, "certified");
        lua_pushinteger(L, static_cast<lua_Integer>(it.n_irreducible)); lua_setfield(L, -2, "n_irreducible");
        lua_pushnumber(L, it.rho_nc);                        lua_setfield(L, -2, "rho_nc");
        lua_pushboolean(L, it.pisot ? 1 : 0);                lua_setfield(L, -2, "pisot");
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

// wide_random(target, K_max, seed, max_pairs, max_len, alphabet_size)
//   -> list of instances.
// Like survey.random, but with entries sampled from 0..K_max
// (explicit) instead of the narrow default_K_for_n ± 1.  The wider
// K_max may turn up more λ(G_B) < β candidates like rnd13 --
// DIRECTION_AND_OPEN_THREADS.md thread B "Wider random search".
// alphabet_size: must be > 0 (default 4).
int l_survey_wide_random(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    std::size_t target = static_cast<std::size_t>(lua_tointeger(L, 1));
    int K_max = 7;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        luaL_checktype(L, 2, LUA_TNUMBER);
        K_max = static_cast<int>(lua_tointeger(L, 2));
    }
    std::uint32_t seed = 11;
    std::size_t max_pairs_cert = 8000;
    std::size_t max_len_cert   = 60000;
    int alphabet_size = 4;
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        seed = static_cast<std::uint32_t>(lua_tointeger(L, 3));
    }
    if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
        luaL_checktype(L, 4, LUA_TNUMBER);
        max_pairs_cert = static_cast<std::size_t>(lua_tointeger(L, 4));
    }
    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
        luaL_checktype(L, 5, LUA_TNUMBER);
        max_len_cert = static_cast<std::size_t>(lua_tointeger(L, 5));
    }
    if (lua_gettop(L) >= 6 && !lua_isnil(L, 6)) {
        luaL_checktype(L, 6, LUA_TNUMBER);
        alphabet_size = static_cast<int>(lua_tointeger(L, 6));
    }
    auto instances = ravel::wide_random_pisot_survey(
        target, K_max, seed, max_pairs_cert, max_len_cert, max_pairs_cert, max_len_cert,
        /*max_trials=*/16000, alphabet_size);
    lua_createtable(L, static_cast<int>(instances.size()), 0);
    for (std::size_t i = 0; i < instances.size(); ++i) {
        const auto& it = instances[i];
        lua_createtable(L, 0, 9);
        lua_pushstring(L, it.name.c_str());                  lua_setfield(L, -2, "name");
        lua_pushnumber(L, it.beta);                          lua_setfield(L, -2, "beta");
        lua_pushnumber(L, it.b2);                            lua_setfield(L, -2, "b2");
        lua_pushboolean(L, it.certified ? 1 : 0);            lua_setfield(L, -2, "certified");
        lua_pushinteger(L, static_cast<lua_Integer>(it.n_irreducible)); lua_setfield(L, -2, "n_irreducible");
        lua_pushnumber(L, it.rho_nc);                        lua_setfield(L, -2, "rho_nc");
        lua_pushboolean(L, it.pisot ? 1 : 0);                lua_setfield(L, -2, "pisot");
        auto cp = ravel::charpoly_int(it.M);
        long long abs_det = cp.empty() ? 0 : std::llabs(cp.back());
        lua_pushinteger(L, static_cast<lua_Integer>(abs_det)); lua_setfield(L, -2, "abs_det");
        lua_createtable(L, static_cast<int>(it.M.size()), 0);
        for (std::size_t r = 0; r < it.M.size(); ++r) {
            lua_createtable(L, static_cast<int>(it.M[r].size()), 0);
            for (std::size_t c = 0; c < it.M[r].size(); ++c) {
                lua_pushinteger(L, static_cast<lua_Integer>(it.M[r][c]));
                lua_rawseti(L, -2, static_cast<int>(c) + 1);
            }
            lua_rawseti(L, -2, static_cast<int>(r) + 1);
        }
        lua_setfield(L, -2, "matrix");
        // Push the substitution as a list-of-lists (image(j) = word j).
        lua_createtable(L, static_cast<int>(it.sigma.size()), 0);
        for (std::size_t j = 0; j < it.sigma.size(); ++j) {
            lua_createtable(L, static_cast<int>(it.sigma[j].size()), 0);
            for (std::size_t k = 0; k < it.sigma[j].size(); ++k) {
                lua_pushinteger(L, static_cast<lua_Integer>(it.sigma[j][k]));
                lua_rawseti(L, -2, static_cast<int>(k) + 1);
            }
            lua_rawseti(L, -2, static_cast<int>(j) + 1);
        }
        lua_setfield(L, -2, "sigma");
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

// pf_ceiling_running(f, lmax, lmin) -> array of running min
// of the running max of |L * f[m] mod 1 - 0.5|, in size lmax+1.
// Mirrors python/pisot_survey.py::ceiling_running.
int l_survey_ceiling(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    std::vector<double> f;
    const std::size_t n = static_cast<std::size_t>(lua_rawlen(L, 1));
    f.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<int>(i) + 1);
        f.push_back(lua_tonumber(L, -1));
        lua_pop(L, 1);
    }
    luaL_checktype(L, 2, LUA_TNUMBER);
    std::size_t lmax = static_cast<std::size_t>(lua_tointeger(L, 2));
    std::size_t lmin = 33;
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        lmin = static_cast<std::size_t>(lua_tointeger(L, 3));
    }
    auto r = ravel::pf_ceiling_running(f, lmax, lmin);
    lua_createtable(L, static_cast<int>(r.size()), 0);
    for (std::size_t i = 0; i < r.size(); ++i) {
        lua_pushnumber(L, r[i]);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

// pf_right(M) -> Perron frequencies (right eigenvector normalized
// to sum 1).
int l_survey_pf_right(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    std::vector<std::vector<long long>> M;
    const std::size_t n = static_cast<std::size_t>(lua_rawlen(L, 1));
    M.assign(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<int>(i) + 1);
        for (std::size_t j = 0; j < n; ++j) {
            lua_rawgeti(L, -1, static_cast<int>(j) + 1);
            M[i][j] = static_cast<long long>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    auto f = ravel::pf_right(M);
    lua_createtable(L, static_cast<int>(f.size()), 0);
    for (std::size_t i = 0; i < f.size(); ++i) {
        lua_pushnumber(L, f[i]);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

// classify_matrix(M) -> { beta, b2, pisot, irred }
int l_survey_classify(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    std::vector<std::vector<long long>> M;
    const std::size_t n = static_cast<std::size_t>(lua_rawlen(L, 1));
    M.assign(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<int>(i) + 1);
        for (std::size_t j = 0; j < n; ++j) {
            lua_rawgeti(L, -1, static_cast<int>(j) + 1);
            M[i][j] = static_cast<long long>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    auto c = ::ravel::classify_matrix_spectral(M);
    lua_createtable(L, 0, 4);
    lua_pushnumber(L, c.beta);   lua_setfield(L, -2, "beta");
    lua_pushnumber(L, c.b2);     lua_setfield(L, -2, "b2");
    lua_pushboolean(L, c.pisot ? 1 : 0); lua_setfield(L, -2, "pisot");
    lua_pushboolean(L, c.irred ? 1 : 0); lua_setfield(L, -2, "irred");
    return 1;
}

const luaL_Reg cyclotomic_funcs[] = {
    {"zero",       l_cyclotomic_zero},   {"one",        l_cyclotomic_one},
    {"d",          l_cyclotomic_d},      {"d2",         l_cyclotomic_d2},
    {"d3",         l_cyclotomic_d3},     {"make",       l_cyclotomic_make},
    {"add",        l_cyclotomic_add},    {"sub",        l_cyclotomic_sub},
    {"mul",        l_cyclotomic_mul},    {"neg",        l_cyclotomic_neg},
    {"scale",      l_cyclotomic_scale},  {"eq",         l_cyclotomic_eq},
    {"tostring",   l_cyclotomic_tostring},{"to_xy",    l_cyclotomic_to_xy},
    {nullptr, nullptr}
};

const luaL_Reg constants_funcs[] = {
    {"half",       l_const_half},       {"one",        l_const_one},
    {"zero",       l_const_zero},       {"sqrt3over2", l_const_sqrt3over2},
    {"d_x",        l_const_dx},         {"d_y",        l_const_dy},
    {"d2_x",       l_const_d2x},        {"d2_y",       l_const_d2y},
    {"d3_x",       l_const_d3x},        {"d3_y",       l_const_d3y},
    {nullptr, nullptr}
};

const luaL_Reg spectral_funcs[] = {
    {"invariants_2x2", l_spectral_2x2},
    {"invariants_3x3", l_spectral_3x3},
    {"charpoly_int", l_spectral_charpoly_int},
    {"qbeta_eigenvalue", l_spectral_qbeta_eigenvalue},
    {nullptr, nullptr}
};

const luaL_Reg tilt_funcs[] = {
    {"tsirelson_bound",  l_tilt_tsirelson},
    {"kl2_bits",         l_tilt_kl2},
    {"hb_mi_floor",      l_tilt_hb_floor},
    {"correlator",       l_tilt_correlator},
    {"silver_cost_angle",l_tilt_silver_cost},
    {nullptr, nullptr}
};

const luaL_Reg subst_funcs[] = {
    {"new",             l_subst_new},
    {"apply_once",      l_subst_apply_once},
    {"expand",          l_subst_expand},
    {"matrix",          l_subst_matrix},
    {"alphabet_size",   l_subst_alpha_size},
    {nullptr, nullptr}
};

const luaL_Reg return_phase_funcs[] = {
    {"build", l_return_phase_build},
    {nullptr, nullptr}
};

const luaL_Reg balanced_pair_funcs[] = {
    {"certify", l_bp_certify},
    {"rho_nc",  l_bp_rho_nc},
    {"analyze_dump", l_bp_analyze_dump},
    {"analyze_dump_text", l_bp_analyze_dump_text},
    {nullptr, nullptr}
};

int push_fibonacci_selection_result(
        lua_State* L, const ravel::FibonacciSelectionResult& result) {
    lua_createtable(L, 0, 4);
    lua_createtable(L, 4, 0);
    for (std::size_t i = 0; i < 4; ++i) {
        lua_pushnumber(L, result.correlators[i]);
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    lua_setfield(L, -2, "correlators");
    lua_createtable(L, 4, 0);
    for (std::size_t i = 0; i < 4; ++i) {
        lua_pushnumber(L, result.accepted_measure[i]);
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    lua_setfield(L, -2, "accepted_measure");
    lua_pushnumber(L, result.chsh);
    lua_setfield(L, -2, "chsh");
    lua_pushinteger(L, static_cast<lua_Integer>(result.partition_cells));
    lua_setfield(L, -2, "partition_cells");
    return 1;
}

int l_fibonacci_selection_exact_quantum(lua_State* L) {
    lua_Integer d1 = luaL_checkinteger(L, 1);
    lua_Integer d2 = luaL_checkinteger(L, 2);
    double shift = luaL_checknumber(L, 3);
    if (d1 <= 0 || d2 <= 0)
        return luaL_error(L, "selection strides must be positive");
    ravel::FibonacciSelectionLimits limits;
    if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
        lua_Integer cap = luaL_checkinteger(L, 4);
        if (cap <= 0) return luaL_error(L, "partition cap must be positive");
        limits.max_partition_cells = static_cast<std::size_t>(cap);
    }
    try {
        auto result = ravel::fibonacci_selection_exact(
            static_cast<std::size_t>(d1), static_cast<std::size_t>(d2),
            shift, ravel::quantum_chsh_targets(), limits);
        return push_fibonacci_selection_result(L, result);
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

int l_fibonacci_selection_exact(lua_State* L) {
    lua_Integer d1 = luaL_checkinteger(L, 1);
    lua_Integer d2 = luaL_checkinteger(L, 2);
    double shift = luaL_checknumber(L, 3);
    if (d1 <= 0 || d2 <= 0)
        return luaL_error(L, "selection strides must be positive");
    luaL_checktype(L, 4, LUA_TTABLE);
    std::array<double, 4> targets{};
    for (std::size_t i = 0; i < targets.size(); ++i) {
        lua_rawgeti(L, 4, static_cast<lua_Integer>(i + 1));
        targets[i] = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        if (!std::isfinite(targets[i]) ||
                targets[i] < -1.0 || targets[i] > 1.0)
            return luaL_error(L, "target correlators must be finite and in [-1,1]");
    }
    ravel::FibonacciSelectionLimits limits;
    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
        lua_Integer cap = luaL_checkinteger(L, 5);
        if (cap <= 0) return luaL_error(L, "partition cap must be positive");
        limits.max_partition_cells = static_cast<std::size_t>(cap);
    }
    try {
        auto result = ravel::fibonacci_selection_exact(
            static_cast<std::size_t>(d1), static_cast<std::size_t>(d2),
            shift, targets, limits);
        return push_fibonacci_selection_result(L, result);
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

const luaL_Reg fibonacci_selection_funcs[] = {
    {"exact", l_fibonacci_selection_exact},
    {"exact_quantum", l_fibonacci_selection_exact_quantum},
    {nullptr, nullptr}
};

int l_fibonacci_finite_run(lua_State* L) {
    lua_Integer n = luaL_checkinteger(L, 1);
    lua_Integer separation = luaL_checkinteger(L, 2);
    lua_Integer d1 = luaL_checkinteger(L, 3);
    lua_Integer d2 = luaL_checkinteger(L, 4);
    if (n <= 0 || separation < 0 || d1 <= 0 || d2 <= 0)
        return luaL_error(L, "finite Fibonacci parameters must be positive");

    std::array<double, 4> targets{};
    const std::array<double, 4>* targets_ptr = nullptr;
    if (!lua_isnoneornil(L, 5)) {
        luaL_checktype(L, 5, LUA_TTABLE);
        for (std::size_t i = 0; i < 4; ++i) {
            lua_rawgeti(L, 5, static_cast<lua_Integer>(i + 1));
            targets[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        targets_ptr = &targets;
    }
    const char* mode_name = luaL_optstring(L, 6, "polarizer");
    ravel::FibonacciOutcomeMode mode;
    if (std::strcmp(mode_name, "polarizer") == 0)
        mode = ravel::FibonacciOutcomeMode::Polarizer;
    else if (std::strcmp(mode_name, "rule30") == 0)
        mode = ravel::FibonacciOutcomeMode::Rule30;
    else
        return luaL_error(L, "mode must be 'polarizer' or 'rule30'");
    lua_Integer steps = luaL_optinteger(L, 7, 16);
    if (steps < 0) return luaL_error(L, "CA steps must be nonnegative");

    std::array<double, 4> baseline{};
    const std::array<double, 4>* baseline_ptr = nullptr;
    if (!lua_isnoneornil(L, 8)) {
        luaL_checktype(L, 8, LUA_TTABLE);
        for (std::size_t i = 0; i < 4; ++i) {
            lua_rawgeti(L, 8, static_cast<lua_Integer>(i + 1));
            baseline[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        baseline_ptr = &baseline;
    }
    ravel::FibonacciFiniteLimits limits;
    if (!lua_isnoneornil(L, 9)) {
        lua_Integer cap = luaL_checkinteger(L, 9);
        if (cap <= 0) return luaL_error(L, "site cap must be positive");
        limits.max_sites = static_cast<std::size_t>(cap);
    }
    try {
        auto r = ravel::fibonacci_selection_finite(
            static_cast<std::size_t>(n),
            static_cast<std::size_t>(separation),
            static_cast<std::size_t>(d1), static_cast<std::size_t>(d2),
            targets_ptr, mode, static_cast<std::size_t>(steps),
            baseline_ptr, limits);
        lua_createtable(L, 0, 5);
        auto push_array = [L](const auto& values, const char* field) {
            lua_createtable(L, 4, 0);
            for (std::size_t i = 0; i < 4; ++i) {
                lua_pushnumber(L, static_cast<lua_Number>(values[i]));
                lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
            }
            lua_setfield(L, -2, field);
        };
        push_array(r.correlators, "correlators");
        push_array(r.kappa, "kappa");
        push_array(r.accepted_count, "accepted_count");
        lua_pushnumber(L, r.chsh);
        lua_setfield(L, -2, "chsh");
        lua_pushinteger(L, static_cast<lua_Integer>(r.sites));
        lua_setfield(L, -2, "sites");
        return 1;
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

const luaL_Reg fibonacci_finite_funcs[] = {
    {"run", l_fibonacci_finite_run},
    {nullptr, nullptr}
};

int l_fibonacci_dynamics_correlations(lua_State* L) {
    lua_Integer sites = luaL_checkinteger(L, 1);
    lua_Integer steps = luaL_checkinteger(L, 2);
    lua_Integer separation = luaL_checkinteger(L, 3);
    if (sites <= 0 || steps < 0 || separation < 0)
        return luaL_error(L, "invalid packed dynamics parameters");
    luaL_checktype(L, 4, LUA_TTABLE);
    const std::size_t nf = lua_rawlen(L, 4);
    std::vector<std::vector<std::size_t>> offsets(nf);
    for (std::size_t i = 0; i < nf; ++i) {
        lua_rawgeti(L, 4, static_cast<lua_Integer>(i + 1));
        luaL_checktype(L, -1, LUA_TTABLE);
        const std::size_t no = lua_rawlen(L, -1);
        offsets[i].reserve(no);
        for (std::size_t j = 0; j < no; ++j) {
            lua_rawgeti(L, -1, static_cast<lua_Integer>(j + 1));
            lua_Integer offset = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
            if (offset < 0) return luaL_error(L, "offsets must be nonnegative");
            offsets[i].push_back(static_cast<std::size_t>(offset));
        }
        lua_pop(L, 1);
    }
    ravel::PackedDynamicsLimits limits;
    if (!lua_isnoneornil(L, 5)) {
        lua_Integer cap = luaL_checkinteger(L, 5);
        if (cap <= 0) return luaL_error(L, "site cap must be positive");
        limits.max_sites = static_cast<std::size_t>(cap);
    }
    if (!lua_isnoneornil(L, 6)) {
        lua_Integer cap = luaL_checkinteger(L, 6);
        if (cap <= 0) return luaL_error(L, "functional cap must be positive");
        limits.max_functionals = static_cast<std::size_t>(cap);
        if (limits.max_functionals >
                std::numeric_limits<std::size_t>::max() /
                    limits.max_functionals)
            return luaL_error(L, "functional cap is too large");
        limits.max_matrix_entries =
            limits.max_functionals * limits.max_functionals;
    }
    try {
        auto state = ravel::packed_fibonacci_word(
            static_cast<std::size_t>(sites));
        ravel::packed_rule30(
            state, static_cast<std::size_t>(steps), limits);
        auto matrix = ravel::packed_parity_correlation_matrix(
            state, offsets, static_cast<std::size_t>(separation), limits);
        lua_createtable(L, 0, 3);
        lua_pushinteger(L, static_cast<lua_Integer>(matrix.sites));
        lua_setfield(L, -2, "sites");
        lua_pushinteger(L, static_cast<lua_Integer>(matrix.function_count));
        lua_setfield(L, -2, "function_count");
        lua_createtable(L, static_cast<int>(nf), 0);
        for (std::size_t i = 0; i < nf; ++i) {
            lua_createtable(L, static_cast<int>(nf), 0);
            for (std::size_t j = 0; j < nf; ++j) {
                lua_pushnumber(L, matrix(i, j));
                lua_rawseti(L, -2, static_cast<lua_Integer>(j + 1));
            }
            lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
        }
        lua_setfield(L, -2, "matrix");
        return 1;
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

int l_fibonacci_dynamics_retarget(lua_State* L) {
    lua_Integer sites = luaL_checkinteger(L, 1);
    lua_Integer steps = luaL_checkinteger(L, 2);
    lua_Integer separation = luaL_checkinteger(L, 3);
    lua_Integer d1 = luaL_checkinteger(L, 4);
    lua_Integer d2 = luaL_checkinteger(L, 5);
    if (sites <= 0 || steps < 0 || separation < 0 || d1 <= 0 || d2 <= 0)
        return luaL_error(L, "invalid packed retarget parameters");
    luaL_checktype(L, 6, LUA_TTABLE);
    std::array<std::vector<std::size_t>, 4> offsets;
    for (std::size_t i = 0; i < 4; ++i) {
        lua_rawgeti(L, 6, static_cast<lua_Integer>(i + 1));
        luaL_checktype(L, -1, LUA_TTABLE);
        const std::size_t no = lua_rawlen(L, -1);
        for (std::size_t j = 0; j < no; ++j) {
            lua_rawgeti(L, -1, static_cast<lua_Integer>(j + 1));
            lua_Integer offset = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
            if (offset < 0) return luaL_error(L, "offsets must be nonnegative");
            offsets[i].push_back(static_cast<std::size_t>(offset));
        }
        lua_pop(L, 1);
    }
    auto read_four_numbers = [L](int index) {
        std::array<double, 4> result{};
        luaL_checktype(L, index, LUA_TTABLE);
        for (std::size_t i = 0; i < 4; ++i) {
            lua_rawgeti(L, index, static_cast<lua_Integer>(i + 1));
            result[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        return result;
    };
    const auto targets = read_four_numbers(7);
    const auto baseline = read_four_numbers(8);
    std::array<int, 4> signs = {1, 1, 1, -1};
    if (!lua_isnoneornil(L, 9)) {
        luaL_checktype(L, 9, LUA_TTABLE);
        for (std::size_t i = 0; i < 4; ++i) {
            lua_rawgeti(L, 9, static_cast<lua_Integer>(i + 1));
            signs[i] = static_cast<int>(luaL_checkinteger(L, -1));
            lua_pop(L, 1);
        }
    }
    ravel::PackedDynamicsLimits limits;
    if (!lua_isnoneornil(L, 10)) {
        lua_Integer cap = luaL_checkinteger(L, 10);
        if (cap <= 0) return luaL_error(L, "site cap must be positive");
        limits.max_sites = static_cast<std::size_t>(cap);
    }
    try {
        auto state = ravel::packed_fibonacci_word(
            static_cast<std::size_t>(sites));
        ravel::packed_rule30(
            state, static_cast<std::size_t>(steps), limits);
        auto r = ravel::packed_parity_retarget(
            state, offsets, static_cast<std::size_t>(separation),
            static_cast<std::size_t>(d1), static_cast<std::size_t>(d2),
            targets, baseline, signs, limits);
        lua_createtable(L, 0, 4);
        auto push_array = [L](const auto& values, const char* field) {
            lua_createtable(L, 4, 0);
            for (std::size_t i = 0; i < 4; ++i) {
                lua_pushnumber(L, static_cast<lua_Number>(values[i]));
                lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
            }
            lua_setfield(L, -2, field);
        };
        push_array(r.correlators, "correlators");
        push_array(r.kappa, "kappa");
        push_array(r.accepted_count, "accepted_count");
        lua_pushnumber(L, r.signed_chsh);
        lua_setfield(L, -2, "signed_chsh");
        return 1;
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

int l_fibonacci_dynamics_local_windows(lua_State* L) {
    lua_Integer sites = luaL_checkinteger(L, 1);
    lua_Integer radius = luaL_checkinteger(L, 2);
    lua_Integer separation = luaL_checkinteger(L, 3);
    lua_Integer d1 = luaL_checkinteger(L, 4);
    lua_Integer d2 = luaL_checkinteger(L, 5);
    if (sites <= 0 || radius <= 0 || separation < 0 || d1 <= 0 || d2 <= 0)
        return luaL_error(L, "invalid local-window parameters");
    luaL_checktype(L, 6, LUA_TTABLE);
    std::array<double, 4> targets{};
    for (std::size_t i = 0; i < 4; ++i) {
        lua_rawgeti(L, 6, static_cast<lua_Integer>(i + 1));
        targets[i] = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    ravel::PackedDynamicsLimits limits;
    if (!lua_isnoneornil(L, 7)) {
        lua_Integer cap = luaL_checkinteger(L, 7);
        if (cap <= 0) return luaL_error(L, "site cap must be positive");
        limits.max_sites = static_cast<std::size_t>(cap);
    }
    try {
        const auto r = ravel::packed_local_window_retarget(
            static_cast<std::size_t>(sites),
            static_cast<std::size_t>(radius),
            static_cast<std::size_t>(separation),
            static_cast<std::size_t>(d1),
            static_cast<std::size_t>(d2), targets, limits);
        lua_createtable(L, 0, 5);
        auto push_array = [L](const auto& values, const char* field) {
            lua_createtable(L, 4, 0);
            for (std::size_t i = 0; i < 4; ++i) {
                lua_pushnumber(L, static_cast<lua_Number>(values[i]));
                lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
            }
            lua_setfield(L, -2, field);
        };
        push_array(r.baseline, "baseline");
        push_array(r.retargeted.correlators, "correlators");
        push_array(r.retargeted.kappa, "kappa");
        push_array(r.retargeted.accepted_count, "accepted_count");
        lua_pushnumber(L, r.baseline_chsh);
        lua_setfield(L, -2, "baseline_chsh");
        lua_pushnumber(L, r.retargeted.signed_chsh);
        lua_setfield(L, -2, "signed_chsh");
        return 1;
    } catch (const std::exception& error) {
        return luaL_error(L, "%s", error.what());
    }
}

const luaL_Reg fibonacci_dynamics_funcs[] = {
    {"correlation_matrix", l_fibonacci_dynamics_correlations},
    {"retarget", l_fibonacci_dynamics_retarget},
    {"local_windows", l_fibonacci_dynamics_local_windows},
    {nullptr, nullptr}
};

const luaL_Reg thermometer_funcs[] = {
    {"measure", l_thermometer},
    {nullptr, nullptr}
};

const luaL_Reg barge_funcs[] = {
    {"flags", l_barge_flags},
    {nullptr, nullptr}
};

const luaL_Reg survey_funcs[] = {
    {"random",          l_survey_random},
    {"wide_random",     l_survey_wide_random},
    {"ceiling_running", l_survey_ceiling},
    {"pf_right",        l_survey_pf_right},
    {"classify_matrix", l_survey_classify},
    {nullptr, nullptr}
};

// ---------- contact_boundary ----------
//
// ravel.contact_boundary.compute(subst, beta, b2, d_cont) -> record.
//
//   subst  : Lua table { word1, word2, ... } (1-indexed lists of ints;
//            alphabet size = #words; entry alphabet is [0, d-1]).
//   beta   : Pisot dominant eigenvalue (precomputed by classify_matrix).
//   b2     : |second-largest-in-magnitude eigenvalue| (precomputed).
//   d_cont : Lua table of { i, {x_0, x_1, ...}, j } triples (0-indexed)
//            giving the seed D_cont for the backward-closure.
//
// Returns a Lua table with fields:
//   alphabet_size, beta, b2,
//   d_cont_size, pre_contact_size, contact_size, signed_contact_size,
//   boundary_size, convergence_rounds, converged, closure_stopped_early,
//   bp_rho_nc, boundary_eigenvalue, conjecture_status,
//   boundary_nodes = { { i, {x_0, ..., x_{d-1}}, j }, ... }.
//
// IMPORTANT: `bp_rho_nc` and `boundary_eigenvalue` are reported as
// DISTINCT numbers; the conjecture that they are equal is NOT a
// theorem for the November-2025 family (for σ_1 they differ:
// 2.286 vs 1.746).
// The `conjecture_status` field surfaces this honestly.  Never
// treat the equality as fact.
int l_contact_boundary_compute(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    double beta = lua_tonumber(L, 2);
    double b2 = lua_tonumber(L, 3);

    std::size_t max_rho_pairs = 20000;
    std::size_t max_rho_len = 60000;
    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
        luaL_checktype(L, 5, LUA_TNUMBER);
        max_rho_pairs = static_cast<std::size_t>(lua_tointeger(L, 5));
    }
    if (lua_gettop(L) >= 6 && !lua_isnil(L, 6)) {
        luaL_checktype(L, 6, LUA_TNUMBER);
        max_rho_len = static_cast<std::size_t>(lua_tointeger(L, 6));
    }

    // d_cont is a list of { i, x_vec, j } triples.
    luaL_checktype(L, 4, LUA_TTABLE);
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    const std::size_t n_dc = static_cast<std::size_t>(lua_rawlen(L, 4));
    d_cont.reserve(n_dc);
    for (std::size_t i = 0; i < n_dc; ++i) {
        lua_rawgeti(L, 4, static_cast<int>(i) + 1);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return luaL_error(L, "d_cont[%d] is not a table",
                              static_cast<int>(i) + 1);
        }
        // [i, x_vec, j]
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        lua_rawgeti(L, -3, 3);
        if (!lua_isnumber(L, -3) || !lua_istable(L, -2)
                || !lua_isnumber(L, -1)) {
            lua_pop(L, 4);
            return luaL_error(L,
                "d_cont[%d] must be { int, table, int }",
                static_cast<int>(i) + 1);
        }
        long long ii = static_cast<long long>(lua_tointeger(L, -3));
        std::vector<long long> xv;
        const std::size_t xn =
            static_cast<std::size_t>(lua_rawlen(L, -2));
        xv.reserve(xn);
        for (std::size_t k = 0; k < xn; ++k) {
            lua_rawgeti(L, -2, static_cast<int>(k) + 1);
            xv.push_back(static_cast<long long>(lua_tointeger(L, -1)));
            lua_pop(L, 1);
        }
        long long jj = static_cast<long long>(lua_tointeger(L, -1));
        d_cont.emplace_back(ii, std::move(xv), jj);
        lua_pop(L, 4);  // pop the [i,x,j] table plus 3 reads
    }

    ravel::ContactBoundaryReport rep;
    try {
        ravel::ContactBoundaryLimits limits;
        limits.max_rho_pairs = max_rho_pairs;
        limits.max_rho_len = max_rho_len;
        rep = ravel::compute_contact_boundary_dispatch(
            r, beta, b2, d_cont, limits);
    } catch (const std::invalid_argument& e) {
        return luaL_error(L, "contact_boundary.compute: %s", e.what());
    }

    lua_createtable(L, 0, 14);
    lua_pushinteger(L, static_cast<lua_Integer>(rep.alphabet_size));
    lua_setfield(L, -2, "alphabet_size");
    lua_pushnumber(L, rep.beta);
    lua_setfield(L, -2, "beta");
    lua_pushnumber(L, rep.b2);
    lua_setfield(L, -2, "b2");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.d_cont_size));
    lua_setfield(L, -2, "d_cont_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.pre_contact_size));
    lua_setfield(L, -2, "pre_contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.contact_size));
    lua_setfield(L, -2, "contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.signed_contact_size));
    lua_setfield(L, -2, "signed_contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.boundary_size));
    lua_setfield(L, -2, "boundary_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.convergence_rounds));
    lua_setfield(L, -2, "convergence_rounds");
    lua_pushboolean(L, rep.converged ? 1 : 0);
    lua_setfield(L, -2, "converged");
    lua_pushboolean(L, rep.closure_stopped_early ? 1 : 0);
    lua_setfield(L, -2, "closure_stopped_early");
    lua_pushboolean(L, rep.corona_capped ? 1 : 0);
    lua_setfield(L, -2, "corona_capped");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.max_a_size_reached));
    lua_setfield(L, -2, "max_a_size_reached");
    lua_pushnumber(L, rep.bp_rho_nc);
    lua_setfield(L, -2, "bp_rho_nc");
    lua_pushnumber(L, rep.boundary_eigenvalue);
    lua_setfield(L, -2, "boundary_eigenvalue");
    lua_pushstring(L, rep.conjecture_status.c_str());
    lua_setfield(L, -2, "conjecture_status");
    // boundary_nodes: list of { i, x_vec, j } triples.
    lua_createtable(L, static_cast<int>(rep.boundary_nodes.size()), 0);
    for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
        const auto& [ii, xv, jj] = rep.boundary_nodes[i];
        lua_createtable(L, 3, 0);
        lua_pushinteger(L, static_cast<lua_Integer>(ii));
        lua_rawseti(L, -2, 1);
        lua_createtable(L, static_cast<int>(xv.size()), 0);
        for (std::size_t k = 0; k < xv.size(); ++k) {
            lua_pushinteger(L, static_cast<lua_Integer>(xv[k]));
            lua_rawseti(L, -2, static_cast<int>(k) + 1);
        }
        lua_rawseti(L, -2, 2);
        lua_pushinteger(L, static_cast<lua_Integer>(jj));
        lua_rawseti(L, -2, 3);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "boundary_nodes");

    // gb_matrix: G_B adjacency matrix as a flat list-of-lists
    // (rep.gb_matrix[i][j] in {0, 1}).  Optional: only present
    // when the underlying pipeline recorded it (it always does
    // for non-empty boundary).  Exposed so Lua callers can run
    // their own graph analysis (automorphism group size, spectral
    // gap of the boundary graph, etc.) without going through C++.
    if (!rep.gb_matrix.empty()) {
        lua_createtable(L, static_cast<int>(rep.gb_matrix.size()), 0);
        for (std::size_t i = 0; i < rep.gb_matrix.size(); ++i) {
            lua_createtable(L, static_cast<int>(rep.gb_matrix[i].size()), 0);
            for (std::size_t j = 0; j < rep.gb_matrix[i].size(); ++j) {
                lua_pushinteger(L, static_cast<lua_Integer>(rep.gb_matrix[i][j]));
                lua_rawseti(L, -2, static_cast<int>(j) + 1);
            }
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
        lua_setfield(L, -2, "gb_matrix");
    }

    return 1;
}

// Forward declaration: the batch driver is defined further down.
int l_contact_boundary_batch_run(lua_State* L);

// ---------- contact_boundary from_subst (auto-derive D_cont) ----------

// Forward declaration.
int l_contact_boundary_compute(lua_State* L);

// search_D_cont(subst, bound?, beta?) -> list of { i, {x_0, ..., x_{d-1}}, j }
// Run an exhaustive geometric search for D_cont entries in
// [-bound, bound]^d.
//
// Optional `beta` parameter: if supplied, used directly to construct
// the Substitution<d>; otherwise the C++ classify_matrix_spectral is
// used (which has known precision issues on near-Salem 3-letter Pisot
// substitutions -- for those, ALWAYS pass beta explicitly).
int l_d_cont_search(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    long long bound = 2;
    double beta = 0.0;  // 0 means "compute from classify_matrix_spectral"
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        luaL_checktype(L, 2, LUA_TNUMBER);
        bound = static_cast<long long>(lua_tointeger(L, 2));
    }
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        beta = lua_tonumber(L, 3);
    }

    // Run the d-specific search and push results.
    auto push_results_2 = [&](auto subst_2) {
        auto found = ravel::search_D_cont<2>(subst_2, bound);
        lua_createtable(L, static_cast<int>(found.size()), 0);
        for (std::size_t i = 0; i < found.size(); ++i) {
            lua_createtable(L, 3, 0);
            lua_pushinteger(L, static_cast<lua_Integer>(found[i].i));
            lua_rawseti(L, -2, 1);
            lua_createtable(L, 2, 0);
            for (std::size_t k = 0; k < 2; ++k) {
                lua_pushinteger(L, static_cast<lua_Integer>(found[i].x[k]));
                lua_rawseti(L, -2, static_cast<int>(k) + 1);
            }
            lua_rawseti(L, -2, 2);
            lua_pushinteger(L, static_cast<lua_Integer>(found[i].j));
            lua_rawseti(L, -2, 3);
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
    };
    auto push_results_3 = [&](auto subst_3) {
        auto found = ravel::search_D_cont<3>(subst_3, bound);
        lua_createtable(L, static_cast<int>(found.size()), 0);
        for (std::size_t i = 0; i < found.size(); ++i) {
            lua_createtable(L, 3, 0);
            lua_pushinteger(L, static_cast<lua_Integer>(found[i].i));
            lua_rawseti(L, -2, 1);
            lua_createtable(L, 3, 0);
            for (std::size_t k = 0; k < 3; ++k) {
                lua_pushinteger(L, static_cast<lua_Integer>(found[i].x[k]));
                lua_rawseti(L, -2, static_cast<int>(k) + 1);
            }
            lua_rawseti(L, -2, 2);
            lua_pushinteger(L, static_cast<lua_Integer>(found[i].j));
            lua_rawseti(L, -2, 3);
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
    };
    auto push_results_4 = [&](auto subst_4) {
        auto found = ravel::search_D_cont<4>(subst_4, bound);
        lua_createtable(L, static_cast<int>(found.size()), 0);
        for (std::size_t i = 0; i < found.size(); ++i) {
            lua_createtable(L, 3, 0);
            lua_pushinteger(L, static_cast<lua_Integer>(found[i].i));
            lua_rawseti(L, -2, 1);
            lua_createtable(L, 4, 0);
            for (std::size_t k = 0; k < 4; ++k) {
                lua_pushinteger(L, static_cast<lua_Integer>(found[i].x[k]));
                lua_rawseti(L, -2, static_cast<int>(k) + 1);
            }
            lua_rawseti(L, -2, 2);
            lua_pushinteger(L, static_cast<lua_Integer>(found[i].j));
            lua_rawseti(L, -2, 3);
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
    };

    // If beta wasn't supplied, compute it from classify_matrix_spectral.
    if (beta == 0.0) {
        std::vector<std::vector<long long>> M(r.alphabet_size(),
            std::vector<long long>(r.alphabet_size(), 0));
        for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
            for (auto letter : r.image(c)) {
                M[static_cast<std::size_t>(letter)][c] += 1;
            }
        }
        auto cls = ::ravel::classify_matrix_spectral(M);
        beta = cls.beta;
        // NB: even if cls.pisot is false, we proceed (the
        // downstream search_D_cont only needs the substitution
        // rule and its (double-precision) right eigenvector).
    }

    switch (r.alphabet_size()) {
    case 2: {
        auto subst = ::ravel::make_substitution<2>(r, beta);
        push_results_2(subst);
        return 1;
    }
    case 3: {
        auto subst = ::ravel::make_substitution<3>(r, beta);
        push_results_3(subst);
        return 1;
    }
    case 4: {
        auto subst = ::ravel::make_substitution<4>(r, beta);
        push_results_4(subst);
        return 1;
    }
    default:
        return luaL_error(L,
            "search_D_cont: alphabet size %zu not supported (only d=2,3,4)",
            r.alphabet_size());
    }
}

// from_subst(subst, bound?, max_rho_pairs?, max_rho_len?) -> record.
// Same schema as contact_boundary.compute, but D_cont is derived
// automatically via search_D_cont(bound=2).  The user only supplies
// the substitution.
//
// Optional max_rho_pairs / max_rho_len cap the BP-rho_nc BFS so
// batch runs over near-Salem or large n_irr Pisot substitutions
// don't take forever.  Defaults: 20000 / 60000 (same as
// compute_contact_boundary).  For batch crunch, try 4000 / 10000.
int l_contact_boundary_from_subst(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    long long bound = 2;
    std::size_t max_rho_pairs = 20000;
    std::size_t max_rho_len = 60000;
    double beta_override = 0.0;  // 0 = compute from C++
    double b2_override = 0.0;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        luaL_checktype(L, 2, LUA_TNUMBER);
        bound = static_cast<long long>(lua_tointeger(L, 2));
    }
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        max_rho_pairs = static_cast<std::size_t>(lua_tointeger(L, 3));
    }
    if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
        luaL_checktype(L, 4, LUA_TNUMBER);
        max_rho_len = static_cast<std::size_t>(lua_tointeger(L, 4));
    }
    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
        luaL_checktype(L, 5, LUA_TNUMBER);
        beta_override = lua_tonumber(L, 5);
    }
    if (lua_gettop(L) >= 6 && !lua_isnil(L, 6)) {
        luaL_checktype(L, 6, LUA_TNUMBER);
        b2_override = lua_tonumber(L, 6);
    }

    double beta, b2;
    if (beta_override > 0.0) {
        beta = beta_override;
        b2 = b2_override;
    } else {
        std::vector<std::vector<long long>> M(r.alphabet_size(),
            std::vector<long long>(r.alphabet_size(), 0));
        for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
            for (auto letter : r.image(c)) {
                M[static_cast<std::size_t>(letter)][c] += 1;
            }
        }
        auto cls = ::ravel::classify_matrix_spectral(M);
        // NB: we don't error out on cls.pisot=false here.  For
        // near-Salem 3-letter Pisot the C++ classification can
        // mis-classify; we still get a usable double-precision
        // Substitution<d> below, even if the classification is wrong.
        beta = cls.beta;
        b2 = cls.b2;
    }

    ravel::ContactBoundaryReport rep;
    try {
        ravel::ContactBoundaryLimits limits;
        limits.max_rho_pairs = max_rho_pairs;
        limits.max_rho_len = max_rho_len;
        rep = ravel::compute_contact_boundary_from_subst_dispatch(
            r, beta, b2, bound, limits);
    } catch (const std::exception& e) {
        return luaL_error(L, "from_subst: %s", e.what());
    }

    // Push the same schema as l_contact_boundary_compute.  Re-use
    // the same logic by dispatching to the existing helper.
    // (We could refactor push_record into a shared helper, but
    // keeping them parallel makes the schema explicit at each
    // entry point.)
    lua_createtable(L, 0, 14);
    lua_pushinteger(L, static_cast<lua_Integer>(rep.alphabet_size));
    lua_setfield(L, -2, "alphabet_size");
    lua_pushnumber(L, rep.beta);
    lua_setfield(L, -2, "beta");
    lua_pushnumber(L, rep.b2);
    lua_setfield(L, -2, "b2");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.d_cont_size));
    lua_setfield(L, -2, "d_cont_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.pre_contact_size));
    lua_setfield(L, -2, "pre_contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.contact_size));
    lua_setfield(L, -2, "contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.signed_contact_size));
    lua_setfield(L, -2, "signed_contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.boundary_size));
    lua_setfield(L, -2, "boundary_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.convergence_rounds));
    lua_setfield(L, -2, "convergence_rounds");
    lua_pushboolean(L, rep.converged ? 1 : 0);
    lua_setfield(L, -2, "converged");
    lua_pushboolean(L, rep.closure_stopped_early ? 1 : 0);
    lua_setfield(L, -2, "closure_stopped_early");
    lua_pushnumber(L, rep.bp_rho_nc);
    lua_setfield(L, -2, "bp_rho_nc");
    lua_pushnumber(L, rep.boundary_eigenvalue);
    lua_setfield(L, -2, "boundary_eigenvalue");
    lua_pushstring(L, rep.conjecture_status.c_str());
    lua_setfield(L, -2, "conjecture_status");
    lua_createtable(L, static_cast<int>(rep.boundary_nodes.size()), 0);
    for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
        const auto& [ii, xv, jj] = rep.boundary_nodes[i];
        lua_createtable(L, 3, 0);
        lua_pushinteger(L, static_cast<lua_Integer>(ii));
        lua_rawseti(L, -2, 1);
        lua_createtable(L, static_cast<int>(xv.size()), 0);
        for (std::size_t k = 0; k < xv.size(); ++k) {
            lua_pushinteger(L, static_cast<lua_Integer>(xv[k]));
            lua_rawseti(L, -2, static_cast<int>(k) + 1);
        }
        lua_rawseti(L, -2, 2);
        lua_pushinteger(L, static_cast<lua_Integer>(jj));
        lua_rawseti(L, -2, 3);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "boundary_nodes");

    // gb_matrix: G_B adjacency matrix (list-of-lists).  Same as
    // the compute path's record above.
    if (!rep.gb_matrix.empty()) {
        lua_createtable(L, static_cast<int>(rep.gb_matrix.size()), 0);
        for (std::size_t i = 0; i < rep.gb_matrix.size(); ++i) {
            lua_createtable(L, static_cast<int>(rep.gb_matrix[i].size()), 0);
            for (std::size_t j = 0; j < rep.gb_matrix[i].size(); ++j) {
                lua_pushinteger(L, static_cast<lua_Integer>(rep.gb_matrix[i][j]));
                lua_rawseti(L, -2, static_cast<int>(j) + 1);
            }
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
        lua_setfield(L, -2, "gb_matrix");
    }
    return 1;
}

// ---------- contact_boundary.analyze (structural invariants) ----------

// analyze(subst, beta?, b2?) -> record of SubstitutionAnalysis.
// Returns structural features of the substitution that we
// hypothesize separate the "conjecture HOLDS" subset from the
// "conjecture FAILS" subset.  See lua/docs/FINDINGS_*.md for
// the analysis built on top of this.
int l_contact_boundary_analyze(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    double beta = 0.0, b2 = 0.0;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        luaL_checktype(L, 2, LUA_TNUMBER);
        beta = lua_tonumber(L, 2);
    }
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        b2 = lua_tonumber(L, 3);
    }
    // If beta/b2 not provided, try classify_matrix_spectral (with
    // the known near-Salem precision caveat; for those, the caller
    // should always pass beta/b2 explicitly).
    if (beta == 0.0) {
        std::vector<std::vector<long long>> M(r.alphabet_size(),
            std::vector<long long>(r.alphabet_size(), 0));
        for (std::size_t c = 0; c < r.alphabet_size(); ++c) {
            for (auto letter : r.image(c)) {
                M[static_cast<std::size_t>(letter)][c] += 1;
            }
        }
        auto cls = ::ravel::classify_matrix_spectral(M);
        beta = cls.beta;
        b2 = cls.b2;
    }

    ravel::SubstitutionAnalysis a =
        ravel::analyze_substitution(r, beta, b2);

    lua_createtable(L, 0, 16);
    lua_pushinteger(L, static_cast<lua_Integer>(a.alphabet_size));
    lua_setfield(L, -2, "alphabet_size");
    lua_pushnumber(L, a.beta);
    lua_setfield(L, -2, "beta");
    lua_pushnumber(L, a.b2);
    lua_setfield(L, -2, "b2");
    lua_pushnumber(L, a.b3);
    lua_setfield(L, -2, "b3");
    lua_pushinteger(L, static_cast<lua_Integer>(a.det_M));
    lua_setfield(L, -2, "det_M");
    lua_pushboolean(L, a.is_pisot ? 1 : 0);
    lua_setfield(L, -2, "is_pisot");
    lua_pushboolean(L, a.is_unimodular ? 1 : 0);
    lua_setfield(L, -2, "is_unimodular");
    lua_pushboolean(L, a.is_primitive ? 1 : 0);
    lua_setfield(L, -2, "is_primitive");
    lua_pushboolean(L, a.is_injective ? 1 : 0);
    lua_setfield(L, -2, "is_injective");
    lua_pushnumber(L, a.pisot_quality);
    lua_setfield(L, -2, "pisot_quality");
    lua_pushboolean(L, a.conformal ? 1 : 0);
    lua_setfield(L, -2, "conformal");
    lua_pushinteger(L, static_cast<lua_Integer>(a.moduli_count));
    lua_setfield(L, -2, "moduli_count");
    lua_pushboolean(L, a.distinct_initials ? 1 : 0);
    lua_setfield(L, -2, "distinct_initials");
    lua_pushboolean(L, a.constant_finals ? 1 : 0);
    lua_setfield(L, -2, "constant_finals");
    lua_pushboolean(L, a.has_constant_factor ? 1 : 0);
    lua_setfield(L, -2, "has_constant_factor");
    // image_lengths
    lua_createtable(L, static_cast<int>(a.image_lengths.size()), 0);
    for (std::size_t i = 0; i < a.image_lengths.size(); ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(a.image_lengths[i]));
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "image_lengths");
    // secondary_moduli
    lua_createtable(L, static_cast<int>(a.secondary_moduli.size()), 0);
    for (std::size_t i = 0; i < a.secondary_moduli.size(); ++i) {
        lua_pushnumber(L, a.secondary_moduli[i]);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "secondary_moduli");
    // constant_factor_positions
    lua_createtable(L,
        static_cast<int>(a.constant_factor_positions.size()), 0);
    for (std::size_t i = 0; i < a.constant_factor_positions.size(); ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(
            a.constant_factor_positions[i]));
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "constant_factor_positions");
    return 1;
}

const luaL_Reg contact_boundary_funcs[] = {
    {"compute",        l_contact_boundary_compute},
    {"from_subst",     l_contact_boundary_from_subst},
    {"search_d_cont",  l_d_cont_search},
    {"analyze",        l_contact_boundary_analyze},
    {"batch_run",      l_contact_boundary_batch_run},
    {nullptr, nullptr}
};

// ---------- contact_boundary batch driver ----------
//
// ravel.contact_boundary.batch_run(rows, out_path) -> list of records.
//
//   rows     : list of { name, subst, beta, b2, d_cont } rows
//              (where d_cont is a list of { i, {x}, j } triples).
//   out_path : optional path to write JSONL.  If nil/omitted, no
//              file is written -- the records come back in memory.
//
// Returns a list of records; each record has the same schema as
// `compute` (alphabet_size, beta, b2, |D_cont|, |C|, |±C|, |G_B|,
// convergence info, BP-rho_nc, lambda(G_B), conjecture_status,
// boundary_nodes) PLUS an `error` field that is non-nil iff the
// row failed (e.g. unsupported alphabet size).  Errors are caught
// per-row so a single bad row doesn't abort the batch.
static void push_contact_boundary_record(
        lua_State* L, const ravel::ContactBoundaryBatchRecord& rec) {
    lua_createtable(L, 0, 3);
    lua_pushstring(L, rec.name.c_str());
    lua_setfield(L, -2, "name");
    if (!rec.error.empty()) {
        lua_pushstring(L, rec.error.c_str());
        lua_setfield(L, -2, "error");
    }
    // Re-use the same record-pushing helper as l_contact_boundary_compute:
    // we re-build the inner table directly here for symmetry.
    const auto& rep = rec.report;
    lua_createtable(L, 0, 14);
    lua_pushinteger(L, static_cast<lua_Integer>(rep.alphabet_size));
    lua_setfield(L, -2, "alphabet_size");
    lua_pushnumber(L, rep.beta);
    lua_setfield(L, -2, "beta");
    lua_pushnumber(L, rep.b2);
    lua_setfield(L, -2, "b2");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.d_cont_size));
    lua_setfield(L, -2, "d_cont_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.pre_contact_size));
    lua_setfield(L, -2, "pre_contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.contact_size));
    lua_setfield(L, -2, "contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.signed_contact_size));
    lua_setfield(L, -2, "signed_contact_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.boundary_size));
    lua_setfield(L, -2, "boundary_size");
    lua_pushinteger(L, static_cast<lua_Integer>(rep.convergence_rounds));
    lua_setfield(L, -2, "convergence_rounds");
    lua_pushboolean(L, rep.converged ? 1 : 0);
    lua_setfield(L, -2, "converged");
    lua_pushboolean(L, rep.closure_stopped_early ? 1 : 0);
    lua_setfield(L, -2, "closure_stopped_early");
    lua_pushnumber(L, rep.bp_rho_nc);
    lua_setfield(L, -2, "bp_rho_nc");
    lua_pushnumber(L, rep.boundary_eigenvalue);
    lua_setfield(L, -2, "boundary_eigenvalue");
    lua_pushstring(L, rep.conjecture_status.c_str());
    lua_setfield(L, -2, "conjecture_status");
    lua_createtable(L, static_cast<int>(rep.boundary_nodes.size()), 0);
    for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
        const auto& [ii, xv, jj] = rep.boundary_nodes[i];
        lua_createtable(L, 3, 0);
        lua_pushinteger(L, static_cast<lua_Integer>(ii));
        lua_rawseti(L, -2, 1);
        lua_createtable(L, static_cast<int>(xv.size()), 0);
        for (std::size_t k = 0; k < xv.size(); ++k) {
            lua_pushinteger(L, static_cast<lua_Integer>(xv[k]));
            lua_rawseti(L, -2, static_cast<int>(k) + 1);
        }
        lua_rawseti(L, -2, 2);
        lua_pushinteger(L, static_cast<lua_Integer>(jj));
        lua_rawseti(L, -2, 3);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    lua_setfield(L, -2, "boundary_nodes");
    lua_setfield(L, -2, "report");
}

int l_contact_boundary_batch_run(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    const std::size_t n_rows =
        static_cast<std::size_t>(lua_rawlen(L, 1));
    std::vector<ravel::ContactBoundaryBatchRow> rows;
    rows.reserve(n_rows);

    for (std::size_t i = 0; i < n_rows; ++i) {
        lua_rawgeti(L, 1, static_cast<int>(i) + 1);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return luaL_error(L,
                "rows[%d] is not a table",
                static_cast<int>(i) + 1);
        }
        // Accept either positional { name, subst, beta, b2, d_cont }
        // at indices 1..5 OR the more readable named keys { name=...,
        // subst=..., beta=..., b2=..., d_cont=... }.  Try named first
        // (matches the typical Lua-side calling convention); fall back
        // to positional.
        auto try_get_field = [&](const char* key) -> bool {
            lua_getfield(L, -1, key);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                return false;
            }
            return true;
        };
        bool used_named = try_get_field("name");
        if (!used_named) {
            // Fall back to positional: pop the nil, try [1].
            lua_rawgeti(L, -1, 1);
            if (!lua_isstring(L, -1)) {
                lua_pop(L, 1);
                return luaL_error(L,
                    "rows[%d] must be { name, subst, beta, b2, d_cont } "
                    "(positional) or { name=..., subst=..., ... } (named)",
                    static_cast<int>(i) + 1);
            }
        }
        // Now stack top is `name` (string).
        std::string name = lua_tostring(L, -1);
        lua_pop(L, 1);

        // subst (table of word tables).
        if (used_named) {
            if (!try_get_field("subst")) {
                lua_pop(L, 1);
                return luaL_error(L, "rows[%d] missing 'subst'",
                    static_cast<int>(i) + 1);
            }
        } else {
            lua_rawgeti(L, -1, 2);
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                return luaL_error(L,
                    "rows[%d].subst is not a table",
                    static_cast<int>(i) + 1);
            }
        }
        auto rule = lua_check_substitution(L, -1);
        lua_pop(L, 1);

        // beta
        if (used_named) {
            if (!try_get_field("beta")) {
                lua_pop(L, 1);
                return luaL_error(L, "rows[%d] missing 'beta'",
                    static_cast<int>(i) + 1);
            }
        } else {
            lua_rawgeti(L, -1, 3);
        }
        if (!lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            return luaL_error(L, "rows[%d].beta is not a number",
                static_cast<int>(i) + 1);
        }
        double beta = lua_tonumber(L, -1);
        lua_pop(L, 1);

        // b2
        if (used_named) {
            if (!try_get_field("b2")) {
                lua_pop(L, 1);
                return luaL_error(L, "rows[%d] missing 'b2'",
                    static_cast<int>(i) + 1);
            }
        } else {
            lua_rawgeti(L, -1, 4);
        }
        if (!lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            return luaL_error(L, "rows[%d].b2 is not a number",
                static_cast<int>(i) + 1);
        }
        double b2 = lua_tonumber(L, -1);
        lua_pop(L, 1);

        // d_cont (list of {i, x_vec, j} tables; may be empty)
        if (used_named) {
            if (!try_get_field("d_cont")) {
                lua_pop(L, 1);
                return luaL_error(L, "rows[%d] missing 'd_cont'",
                    static_cast<int>(i) + 1);
            }
        } else {
            lua_rawgeti(L, -1, 5);
        }
        std::vector<std::tuple<long long,
                               std::vector<long long>,
                               long long>> d_cont;
        if (lua_istable(L, -1)) {
            const std::size_t n_dc =
                static_cast<std::size_t>(lua_rawlen(L, -1));
            d_cont.reserve(n_dc);
            for (std::size_t k = 0; k < n_dc; ++k) {
                lua_rawgeti(L, -1, static_cast<int>(k) + 1);
                if (!lua_istable(L, -1)) {
                    lua_pop(L, 1);
                    return luaL_error(L,
                        "rows[%d].d_cont[%d] is not a table",
                        static_cast<int>(i) + 1,
                        static_cast<int>(k) + 1);
                }
                lua_rawgeti(L, -1, 1);
                lua_rawgeti(L, -2, 2);
                lua_rawgeti(L, -3, 3);
                long long ii =
                    static_cast<long long>(lua_tointeger(L, -3));
                std::vector<long long> xv;
                const std::size_t xn =
                    static_cast<std::size_t>(lua_rawlen(L, -2));
                xv.reserve(xn);
                for (std::size_t m = 0; m < xn; ++m) {
                    lua_rawgeti(L, -2, static_cast<int>(m) + 1);
                    xv.push_back(static_cast<long long>(
                        lua_tointeger(L, -1)));
                    lua_pop(L, 1);
                }
                long long jj =
                    static_cast<long long>(lua_tointeger(L, -1));
                d_cont.emplace_back(ii, std::move(xv), jj);
                lua_pop(L, 4);
            }
        }
        lua_pop(L, 1);  // pop d_cont

        rows.push_back({std::move(name), std::move(rule),
                        beta, b2, std::move(d_cont)});
        lua_pop(L, 1);  // pop the row table
    }

    auto records = ravel::compute_contact_boundary_batch(rows);

    // Optionally write JSONL.
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        const char* path = luaL_checkstring(L, 2);
        FILE* f = std::fopen(path, "w");
        if (!f) return luaL_error(L,
            "cannot open output file: %s", path);
        for (const auto& rec : records) {
            std::fprintf(f,
                "{\"name\":\"%s\","
                "\"alphabet_size\":%zu,"
                "\"beta\":%.17g,"
                "\"b2\":%.17g,"
                "\"d_cont_size\":%zu,"
                "\"pre_contact_size\":%zu,"
                "\"contact_size\":%zu,"
                "\"signed_contact_size\":%zu,"
                "\"boundary_size\":%zu,"
                "\"convergence_rounds\":%d,"
                "\"converged\":%s,"
                "\"closure_stopped_early\":%s,"
                "\"corona_capped\":%s,"
                "\"max_a_size_reached\":%zu,"
                "\"bp_rho_nc\":%.17g,"
                "\"boundary_eigenvalue\":%.17g,"
                "\"conjecture_status\":\"OPEN CONJECTURE\","
                "\"error\":\"%s\"}\n",
                rec.name.c_str(),
                rec.report.alphabet_size,
                rec.report.beta,
                rec.report.b2,
                rec.report.d_cont_size,
                rec.report.pre_contact_size,
                rec.report.contact_size,
                rec.report.signed_contact_size,
                rec.report.boundary_size,
                rec.report.convergence_rounds,
                rec.report.converged ? "true" : "false",
                rec.report.closure_stopped_early ? "true" : "false",
                rec.report.corona_capped ? "true" : "false",
                rec.report.max_a_size_reached,
                rec.report.bp_rho_nc,
                rec.report.boundary_eigenvalue,
                rec.error.c_str());
        }
        std::fclose(f);
    }

    // Push records list.
    lua_createtable(L, static_cast<int>(records.size()), 0);
    for (std::size_t i = 0; i < records.size(); ++i) {
        push_contact_boundary_record(L, records[i]);
        lua_rawseti(L, -2, static_cast<int>(i) + 1);
    }
    return 1;
}

void register_module(lua_State* L, const char* name, const luaL_Reg* funcs) {
    lua_newtable(L);
    luaL_setfuncs(L, funcs, 0);
    lua_setfield(L, -2, name);
}

// ---------- Rauzy fractal (native graph-directed IFS) ----------

// ravel.rauzy_fractal.generate(sigma, n_points, seed)
// -> {beta, points={x,y,z,...}, max_affine_error}.
// Traversal and projection stay native; Lua receives a compact flat
// coordinate buffer suitable for bounded export or rendering.
static int l_rauzy_generate(lua_State* L) {
    // Read sigma.
    if (!lua_istable(L, 1)) return luaL_error(L, "sigma must be a table");
    int n = (int)lua_rawlen(L, 1);
    if (n != 4) return luaL_error(L, "rauzy_fractal.generate only supports d=4");

    std::array<std::vector<std::int8_t>, 4> images;
    for (int i = 0; i < 4; ++i) {
        lua_rawgeti(L, 1, i + 1);
        if (!lua_istable(L, -1)) return luaL_error(L, "sigma[%d] not a list", i + 1);
        int m = (int)lua_rawlen(L, -1);
        images[i].resize(m);
        for (int j = 0; j < m; ++j) {
            lua_rawgeti(L, -1, j + 1);
            images[i][j] = (std::int8_t)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }

    lua_Integer requested = luaL_checkinteger(L, 2);
    if (requested < 1 || requested > 5000000) {
        return luaL_error(L, "n_points must be in [1, 5000000]");
    }
    std::size_t n_points = static_cast<std::size_t>(requested);
    std::uint32_t seed = 42;
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        seed = static_cast<std::uint32_t>(luaL_checkinteger(L, 3));
    }

    ravel::RauzyFractal<4> rf(images);
    auto raw = rf.chaos_game(n_points, seed);
    double uu = 0.0, vu = 0.0;
    for (int k = 0; k < 4; ++k) {
        uu += rf.u()[k] * rf.u()[k];
        vu += rf.v()[k] * rf.u()[k];
    }
    const double expected_height = vu / (rf.beta() - 1.0);
    double max_affine_error = 0.0;

    lua_createtable(L, 0, 3);
    lua_pushnumber(L, rf.beta());
    lua_setfield(L, -2, "beta");
    lua_createtable(L, static_cast<int>(n_points * 3), 0);
    for (std::size_t s = 0; s < raw.size(); ++s) {
        double height = 0.0;
        for (int k = 0; k < 4; ++k) height += raw[s][k] * rf.u()[k];
        max_affine_error =
            std::max(max_affine_error, std::abs(height - expected_height));
        auto pos = rf.project_to_W(raw[s]);
        for (int k = 0; k < 3; ++k) {
            if (!std::isfinite(pos[k])) {
                return luaL_error(L, "Rauzy generator produced non-finite coordinates");
            }
            lua_pushnumber(L, pos[k]);
            lua_rawseti(L, -2, static_cast<lua_Integer>(3 * s + k + 1));
        }
    }
    lua_setfield(L, -2, "points");
    lua_pushnumber(L, max_affine_error);
    lua_setfield(L, -2, "max_affine_error");
    return 1;
}

static const struct luaL_Reg rauzy_fractal_funcs[] = {
    {"generate", l_rauzy_generate},
    {NULL, NULL}
};

// ---------- d_cont_check: standalone is_in_D_cont ----------
//
// Exposes d_cont_check.hpp's geometric D_cont check to Lua so the
// pipeline isn't restricted to the contact_boundary.* entry points:
// callers can hand-pick a candidate (i, x, j) and ask whether it
// satisfies the geometric D_cont definition for the substitution,
// independently of running the full contact-boundary pipeline.
//
// Per docs/DIRECTION_AND_OPEN_THREADS.md thread C, this binding
// fills the gap where d_cont_check.hpp was only reachable via
// `ravel.contact_boundary.compute`.

// is_in_D_cont(subst, candidate, beta?) -> bool
//   subst:     substitution table (1-indexed lists of 0-indexed letters)
//   candidate: { i, {x_0, ..., x_{d-1}}, j }
//   beta:      optional explicit beta; if omitted, computed from
//              the substitution matrix via classify_matrix_spectral.
//   Returns:   true iff the (i, x, j) triple is in D_cont under the
//              geometric definition (in_H_sigma, face-intersection-dim
//              == d-2, and (i < j) at the origin for anti-symmetrization).
static int l_d_cont_check_is_in_d_cont(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    const std::size_t d = r.alphabet_size();

    // Parse candidate = { i, {x_0, ..., x_{d-1}}, j }.
    luaL_checktype(L, 2, LUA_TTABLE);
    if (lua_rawlen(L, 2) != 3)
        return luaL_error(L, "candidate must be { i, {x...}, j } (length 3)");
    lua_rawgeti(L, 2, 1);
    long long i = static_cast<long long>(lua_tointeger(L, -1));
    lua_pop(L, 1);
    lua_rawgeti(L, 2, 3);
    long long j = static_cast<long long>(lua_tointeger(L, -1));
    lua_pop(L, 1);
    lua_rawgeti(L, 2, 2);
    luaL_checktype(L, -1, LUA_TTABLE);
    if (lua_rawlen(L, -1) != d)
        return luaL_error(L, "candidate.x has %d entries but alphabet size is %zu",
                          static_cast<int>(lua_rawlen(L, -1)), d);
    std::array<long long, 8> x_buf{};
    static_assert(8 >= 8, "x_buf covers d up to 8; bump if needed");
    for (std::size_t k = 0; k < d; ++k) {
        lua_rawgeti(L, -1, static_cast<int>(k) + 1);
        x_buf[k] = static_cast<long long>(lua_tointeger(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    // Optional beta.
    double beta = 0.0;
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        beta = lua_tonumber(L, 3);
    }

    bool result = false;
    auto dispatch = [&](auto tag) {
        constexpr std::size_t D = decltype(tag)::value;
        if (beta == 0.0) {
            std::vector<std::vector<long long>> Mvec(d,
                std::vector<long long>(d, 0));
            for (std::size_t c = 0; c < d; ++c)
                for (auto letter : r.image(c))
                    Mvec[static_cast<std::size_t>(letter)][c] += 1;
            beta = ::ravel::classify_matrix_spectral(Mvec).beta;
        }
        auto subst = ::ravel::make_substitution<D>(r, beta);
        typename ::ravel::DCandidate<D> cand{};
        cand.i = i;
        cand.j = j;
        for (std::size_t k = 0; k < d; ++k)
            cand.x[k] = x_buf[k];
        result = ::ravel::is_in_D_cont(subst, cand);
    };

    switch (d) {
    case 2: dispatch(std::integral_constant<std::size_t, 2>{}); break;
    case 3: dispatch(std::integral_constant<std::size_t, 3>{}); break;
    case 4: dispatch(std::integral_constant<std::size_t, 4>{}); break;
    default:
        return luaL_error(L,
            "d_cont_check.is_in_D_cont: alphabet size %zu not supported "
            "(only d=2,3,4)", d);
    }
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

// verify_table(subst, candidates, beta?) -> count
//   Counts how many of the supplied candidates satisfy the
//   geometric D_cont definition.  Mirrors the reference
//   `validate_dcont.py` workflow end-to-end (read the substitution,
//   read the table, count passes).
static int l_d_cont_check_verify_table(lua_State* L) {
    auto r = lua_check_substitution(L, 1);
    const std::size_t d = r.alphabet_size();

    luaL_checktype(L, 2, LUA_TTABLE);
    const std::size_t n_cands = static_cast<std::size_t>(lua_rawlen(L, 2));

    double beta = 0.0;
    if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TNUMBER);
        beta = lua_tonumber(L, 3);
    }

    auto dispatch = [&](auto tag) {
        constexpr std::size_t D = decltype(tag)::value;
        if (beta == 0.0) {
            std::vector<std::vector<long long>> Mvec(d,
                std::vector<long long>(d, 0));
            for (std::size_t c = 0; c < d; ++c)
                for (auto letter : r.image(c))
                    Mvec[static_cast<std::size_t>(letter)][c] += 1;
            beta = ::ravel::classify_matrix_spectral(Mvec).beta;
        }
        auto subst = ::ravel::make_substitution<D>(r, beta);
        std::size_t count = 0;
        for (std::size_t k = 0; k < n_cands; ++k) {
            lua_rawgeti(L, 2, static_cast<int>(k) + 1);
            lua_rawgeti(L, -1, 1);
            long long i = static_cast<long long>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            lua_rawgeti(L, -1, 3);
            long long j = static_cast<long long>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            lua_rawgeti(L, -1, 2);
            if (lua_rawlen(L, -1) != d) {
                lua_pop(L, 2);
                continue;
            }
            std::array<long long, 8> x_buf{};
            for (std::size_t m = 0; m < d; ++m) {
                lua_rawgeti(L, -1, static_cast<int>(m) + 1);
                x_buf[m] = static_cast<long long>(lua_tointeger(L, -1));
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
            typename ::ravel::DCandidate<D> cand{};
            cand.i = i;
            cand.j = j;
            for (std::size_t m = 0; m < d; ++m) cand.x[m] = x_buf[m];
            if (::ravel::is_in_D_cont(subst, cand)) ++count;
            lua_pop(L, 1);
        }
        lua_pushinteger(L, static_cast<lua_Integer>(count));
    };

    switch (d) {
    case 2: dispatch(std::integral_constant<std::size_t, 2>{}); break;
    case 3: dispatch(std::integral_constant<std::size_t, 3>{}); break;
    case 4: dispatch(std::integral_constant<std::size_t, 4>{}); break;
    default:
        return luaL_error(L,
            "d_cont_check.verify_table: alphabet size %zu not supported "
            "(only d=2,3,4)", d);
    }
    return 1;
}

static const luaL_Reg d_cont_check_funcs[] = {
    {"is_in_D_cont", l_d_cont_check_is_in_d_cont},
    {"verify_table", l_d_cont_check_verify_table},
    {nullptr, nullptr}
};

int luaopen_spectre(lua_State* L) {
    lua_newtable(L);
    register_module(L, "cyclotomic",    cyclotomic_funcs);
    register_module(L, "constants",     constants_funcs);
    register_module(L, "spectral",      spectral_funcs);
    register_module(L, "tilt",          tilt_funcs);
    register_module(L, "substitution",  subst_funcs);
    register_module(L, "return_phase",  return_phase_funcs);
    register_module(L, "balanced_pair", balanced_pair_funcs);
    register_module(L, "fibonacci_selection", fibonacci_selection_funcs);
    register_module(L, "fibonacci_finite", fibonacci_finite_funcs);
    register_module(L, "fibonacci_dynamics", fibonacci_dynamics_funcs);
    register_module(L, "thermometer",   thermometer_funcs);
    register_module(L, "barge",         barge_funcs);
    register_module(L, "survey",        survey_funcs);
    register_module(L, "contact_boundary", contact_boundary_funcs);
    register_module(L, "rauzy_fractal", rauzy_fractal_funcs);
    register_module(L, "d_cont_check",  d_cont_check_funcs);
    return 1;
}

}  // namespace

extern "C" int luaopen_spectre_native(lua_State* L) {
    return luaopen_spectre(L);
}
