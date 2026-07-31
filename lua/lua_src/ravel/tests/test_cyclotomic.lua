-- lua/tests/test_cyclotomic.lua
--
-- 12th cyclotomic ring identities. There is no `tests/
-- test_cyclotomic.cpp` in this repository -- this is the only test
-- of `include/ravel/cyclotomic.hpp`'s arithmetic, not a mirror of one.
-- All arithmetic goes through the C++ backend (libspectre_native.so);
-- the test verifies that the ring is closed under multiplication,
-- that d^4 = d^2 - 1 is the minimal polynomial, and that the
-- to_xy() conversion uses the named constants from constants.lua.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local EQ = runner.assert_eq
local NEAR = runner.assert_near
local T = runner.assert_true
local F = runner.assert_false

-- Each test reads the cyclotomic helpers from the ravel namespace
-- initialised by the test main; the namespace is passed in via
-- a module-level variable set in main() below.

local sp  -- the ravel module
local cyc -- cyclotomic helpers
local con -- constants

local function cyclotomic_12_basis_values()
    local zero = cyc.zero()
    local xy = cyc.to_xy(zero)
    NEAR(xy[1], 0.0, 1e-12, "zero x")
    NEAR(xy[2], 0.0, 1e-12, "zero y")

    local one = cyc.one()
    local oxy = cyc.to_xy(one)
    NEAR(oxy[1], 1.0, 1e-12, "one x")
    NEAR(oxy[2], 0.0, 1e-12, "one y")

    local d = cyc.d()
    local dxy = cyc.to_xy(d)
    NEAR(dxy[1], con.D_X, 1e-12, "d x = cos(pi/6)")
    NEAR(dxy[2], con.D_Y, 1e-12, "d y = sin(pi/6)")

    local d2 = cyc.d2()
    local d2xy = cyc.to_xy(d2)
    NEAR(d2xy[1], con.D2_X, 1e-12, "d^2 x = cos(pi/3)")
    NEAR(d2xy[2], con.D2_Y, 1e-12, "d^2 y = sin(pi/3)")

    local d3 = cyc.d3()
    local d3xy = cyc.to_xy(d3)
    NEAR(d3xy[1], con.D3_X, 1e-12, "d^3 x = cos(pi/2) = 0")
    NEAR(d3xy[2], con.D3_Y, 1e-12, "d^3 y = sin(pi/2) = 1")

    local combo = cyc.make(1, 2, 0, 0)
    local cxy = cyc.to_xy(combo)
    NEAR(cxy[1], 1.0 + 2.0 * con.D_X, 1e-12, "1 + 2d x")
    NEAR(cxy[2], 1.0, 1e-12, "1 + 2d y")
end

local function cyclotomic_12_minimal_polynomial()
    local d  = cyc.d()
    local d2 = cyc.mul(d, d)
    local d4 = cyc.mul(d2, d2)
    -- d^4 - d^2 + 1 == 0 is the 12th cyclotomic minimal polynomial.
    local identity = cyc.add(cyc.sub(d4, d2), cyc.one())
    T(cyc.eq(identity, cyc.zero()), "d^4 - d^2 + 1 != 0")
end

local function cyclotomic_12_powers()
    local d  = cyc.d()
    local d2 = cyc.mul(d, d)
    local d3 = cyc.mul(d2, d)
    local d4 = cyc.mul(d2, d2)
    T(cyc.eq(d2, cyc.d2()), "d*d != d^2")
    T(cyc.eq(d3, cyc.d3()), "d*d*d != d^3")
    -- d^4 = d^2 - 1
    local d4_expected = cyc.sub(cyc.d2(), cyc.one())
    T(cyc.eq(d4, d4_expected), "d^4 != d^2 - 1")
    -- d^6 = -1 (since d^6 = (d^2)^3 = d^2 - 1 squared twice... verify)
    local d6 = cyc.mul(d3, d3)
    local d6_expected = cyc.neg(cyc.one())
    T(cyc.eq(d6, d6_expected), "d^6 != -1")
end

local function cyclotomic_12_xy_consistency()
    local lhs = cyc.mul(cyc.make(3, 4, -1, 2), cyc.make(-2, 1, 5, 0))
    local zero = cyc.sub(lhs, lhs)
    T(cyc.eq(zero, cyc.zero()), "lhs - lhs != 0")
    local xy_a = cyc.to_xy(cyc.make(7, -3, 11, 4))
    local sum  = cyc.add(cyc.make(2, 1, 5, 0), cyc.make(5, -4, 6, 4))
    local xy_b = cyc.to_xy(sum)
    NEAR(xy_a[1], xy_b[1], 1e-12, "x equal after decomposition")
    NEAR(xy_a[2], xy_b[2], 1e-12, "y equal after decomposition")
end

local function silver_identity_via_ring()
    -- The C++ cyclotomic ring makes 9 - 6*sqrt(2) expressible as
    -- beta + beta^3 + beta^4 with beta = sqrt(2) - 1; in the
    -- cyclotomic ring this is not directly expressible (we are in
    -- Q(d) not Q(sqrt(2))), so this test just confirms the
    -- cos(x-y) identities that DO use sqrt(2) for the silver mean
    -- work end-to-end via the tilt helpers.
    local inv = sp.spectral.invariants_2x2(2, 1, 1, 0)
    NEAR(inv.beta, 1.0 + math.sqrt(2.0), 1e-10, "silver Perron root")
    NEAR(inv.det_M, -1, 1e-10, "silver det = -1")
    T(inv.invol_safe, "silver is invol-safe")
end

local function tribonacci_spectral()
    -- Tribonacci substitution matrix M (a=0, b=1, c=2):
    -- sigma(a) = ab -> M[0][0] += 1, M[1][0] += 1
    -- sigma(b) = ac -> M[0][1] += 1, M[2][1] += 1
    -- sigma(c) = a  -> M[0][2] += 1
    -- So M = [[1,1,1],[1,0,0],[0,1,0]].  Perron root ≈ 1.839286755214161.
    local M = { {1, 1, 1}, {1, 0, 0}, {0, 1, 0} }
    local inv = sp.spectral.invariants_3x3(M)
    NEAR(inv.beta, 1.839286755214161, 1e-9, "tribonacci Perron root")
    NEAR(inv.det_M, 1, 1e-10, "tribonacci det = 1 (unimodular)")
    T(inv.invol_safe, "tribonacci is invol-safe")
    -- n - 1 inequality: beta2 >= beta^(-1/2)
    T(inv.bound_holds, "tribonacci n-1 inequality holds")
end

local function hall_branciard_floor()
    -- 0.046274 bits from KL_2((2+sqrt(2))/4 || 3/4).
    local floor = sp.tilt.hb_mi_floor()
    NEAR(floor, 0.046273846853407, 1e-12, "HB MI floor")
    -- Tsirelson bound = 2*sqrt(2)
    NEAR(sp.tilt.tsirelson_bound(), 2.0 * math.sqrt(2.0), 1e-12, "Tsirelson bound")
end

local function tilt_correlator_diag()
    -- cos(0) = 1, cos(pi/2) = 0, cos(pi) = -1, cos(3*pi/2) = 0.
    NEAR(sp.tilt.correlator(0, 0),  1.0, 1e-12, "E(0,0)=1")
    NEAR(sp.tilt.correlator(1, 1),  1.0, 1e-12, "E(pi/2, pi/2)=1")
    NEAR(sp.tilt.correlator(2, 2),  1.0, 1e-12, "E(pi, pi)=1")
    NEAR(sp.tilt.correlator(3, 3),  1.0, 1e-12, "E(3pi/2, 3pi/2)=1")
    NEAR(sp.tilt.correlator(0, 2), -1.0, 1e-12, "E(0, pi)=-1")
    NEAR(sp.tilt.correlator(1, 3), -1.0, 1e-12, "E(pi/2, 3pi/2)=-1")
end

local function tilt_silver_cost_at_pi_over_4()
    -- At theta = pi/4, p = (1 + cos(pi/4))/2 = (1 + sqrt(2)/2)/2.
    -- KL_2(p || 3/4) is the per-angle Hall-Branciard cost.
    -- The convention in our closed form returns 0 at the deterministic
    -- endpoints (theta = 0, pi) and is strictly positive in between,
    -- so we test the interior point.
    local cost = sp.tilt.silver_cost_angle(math.pi / 4.0)
    local p = (1.0 + math.cos(math.pi / 4.0)) / 2.0
    local q = 3.0 / 4.0
    local expected = p * math.log(p / q) / math.log(2.0)
                       + (1.0 - p) * math.log((1.0 - p) / (1.0 - q)) / math.log(2.0)
    NEAR(cost, expected, 1e-12, "tilt cost at pi/4 matches closed form")
    T(cost > 0.0, "tilt cost at pi/4 is positive")
end

-- Structural properties of the per-angle Hall-Branciard cost:
--   KL_2((1 + cos theta)/2 || 3/4).
-- These are invariant properties of the closed form, not specific
-- to the 0.046274 floor at S = 2*sqrt(2).  See Hall & Branciard,
-- PRA 102, 052228 (2020) Eq. (29), and the W1 audit in
-- NOTES_W1W2.md [4] (reference technical note).
local function tilt_silver_cost_structural_properties()
    local q = 3.0 / 4.0
    local function cost(theta)
        return sp.tilt.silver_cost_angle(theta)
    end
    -- Deterministic endpoints -> KL = 0
    NEAR(cost(0.0),       0.0, 1e-12, "tilt cost at theta=0 is 0")
    NEAR(cost(math.pi),   0.0, 1e-12, "tilt cost at theta=pi is 0")
    NEAR(cost(2 * math.pi), 0.0, 1e-12, "tilt cost at theta=2*pi is 0")
    -- Symmetry: cost(theta) == cost(-theta)
    for _, theta in ipairs({ 0.1, 0.5, 1.0, 1.5, 2.5 }) do
        NEAR(cost(theta), cost(-theta), 1e-12,
            string.format("tilt cost is even in theta at theta=%.3f", theta))
    end
    -- Interior peak near pi/2 and 3*pi/2
    T(cost(math.pi / 2.0) > 0.2,
        "tilt cost at pi/2 exceeds 0.2 bits (interior peak)")
    T(cost(3 * math.pi / 2.0) > 0.2,
        "tilt cost at 3*pi/2 exceeds 0.2 bits")
    NEAR(cost(math.pi / 2.0), cost(3 * math.pi / 2.0), 1e-12,
        "tilt cost at pi/2 and 3*pi/2 are equal by cos(-t) symmetry")
    -- The HB floor 0.046274 bits sits at p = (2 + sqrt(2))/4, i.e.
    -- cos(theta) = +sqrt(2)/2.  The two solutions in [0, 2*pi] are
    -- theta = pi/4 and theta = 7*pi/4.  Verify these are where
    -- the per-angle closed form meets the HB floor value.
    NEAR(cost(math.pi / 4.0), sp.tilt.hb_mi_floor(), 1e-12,
        "tilt cost at theta=pi/4 == HB floor 0.046274")
    NEAR(cost(7 * math.pi / 4.0), sp.tilt.hb_mi_floor(), 1e-12,
        "tilt cost at theta=7*pi/4 == HB floor 0.046274")
end

local function registry()
    return {
        { name = "cyclotomic_12_basis_values",        fn = cyclotomic_12_basis_values        },
        { name = "cyclotomic_12_minimal_polynomial",  fn = cyclotomic_12_minimal_polynomial  },
        { name = "cyclotomic_12_powers",              fn = cyclotomic_12_powers              },
        { name = "cyclotomic_12_xy_consistency",      fn = cyclotomic_12_xy_consistency      },
        { name = "silver_spectral_invariants",        fn = silver_identity_via_ring          },
        { name = "tribonacci_spectral_invariants",    fn = tribonacci_spectral               },
        { name = "hall_branciard_floor",              fn = hall_branciard_floor              },
        { name = "tilt_correlator_diagonal_values",   fn = tilt_correlator_diag              },
        { name = "tilt_silver_cost_at_theta_pi_over_4", fn = tilt_silver_cost_at_pi_over_4 },
        { name = "tilt_silver_cost_structural_properties", fn = tilt_silver_cost_structural_properties },
    }
end

local function main(spectre_module)
    sp  = spectre_module
    cyc = spectre_module.cyclo
    con = spectre_module.constants
    for _, c in ipairs(registry()) do
        TEST(c.name, c.fn)
    end
    return R.run_all()
end

return { main = main }
