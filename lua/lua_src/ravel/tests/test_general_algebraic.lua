-- lua/ravel/tests/test_general_algebraic.lua
--
-- Tests for the general algebraic path (the "post-8-fixed-ring layer"
-- documented in DIRECTION_AND_OPEN_THREADS.md thread C item 1):
--
--   ravel.spectral.charpoly_int(M)
--     -> { c0, c1, ..., cd }    (lowest-degree first, monic)
--   ravel.spectral.qbeta_eigenvalue(M, c, [k_max])
--     -> { lambda, converged, steps_taken, peak_memory_bytes, error }
--
-- These two bindings close the gap that the legacy 8-named-ring
-- algebraic.hpp leaves: the full 39-substitution and 87-candidate
-- non-unimodular surveys used classify_matrix_spectral (double
-- precision) and only the 11 clean 4-letter Pisot candidates were
-- re-computed bit-exactly via qbeta_dominant_eigenvalue_4.  With
-- charpoly_int(M) + qbeta_eigenvalue(M, c), the full survey can
-- be re-run with bit-exact arithmetic at every matrix size.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local NEAR = runner.assert_near
local T    = runner.assert_true

local sp

-- ---------- charpoly_int: exact integer charpoly of an integer matrix ----------

-- Tribonacci matrix [[1,1,1],[1,0,0],[0,1,0]]
--   charpoly = x^3 - x^2 - x - 1
--   In barge.hpp's descending-order convention: cp[0] = 1, cp[1] = -1,
--   cp[2] = -1, cp[3] = -1.  In LUA (1-indexed): cp[1]=1 (monic),
--   cp[2]=-1, cp[3]=-1, cp[4]=-1 (constant = -1 = -det, since det(M) = 1).
local function charpoly_int_tribonacci()
    local trib = { {1, 1, 1}, {1, 0, 0}, {0, 1, 0} }
    local cp = sp.spectral.charpoly_int(trib)
    T(#cp == 4,
        string.format("tribonacci charpoly has 4 coefficients (got %d)", #cp))
    T(cp[1] == 1, "tribonacci charpoly is monic (cp[1]=1)")
    -- constant = (-1)^3 * det(M) = -1 * 1 = -1
    T(cp[4] == -1, "tribonacci charpoly constant = -1 (= (-1)^n * det)")
    -- x^2 coefficient = -tr(M) = -1
    T(cp[3] == -1, "tribonacci charpoly x^2 coef = -1 = -tr(M)")
    local inv = sp.spectral.invariants_3x3(trib)
    local qe = sp.spectral.qbeta_eigenvalue(trib, cp, 60)
    T(qe.converged,
        string.format("tribonacci qbeta eigenvalue converged (got %d, err=%s)",
            qe.steps_taken, qe.error))
    NEAR(qe.lambda, inv.beta, 1e-9,
        string.format("tribonacci qbeta lambda (%.6f) matches double-precision beta (%.6f)",
            qe.lambda, inv.beta))
end

-- Fibonacci matrix [[1,1],[1,0]]
--   charpoly = x^2 - x - 1
--   In LUA (1-indexed): cp[1]=1 (monic), cp[2]=-1, cp[3]=-1 (constant).
local function charpoly_int_fibonacci()
    local fib = { {1, 1}, {1, 0} }
    local cp = sp.spectral.charpoly_int(fib)
    T(#cp == 3, "fibonacci charpoly has 3 coefficients")
    T(cp[1] == 1, "fibonacci charpoly is monic")
    T(cp[2] == -1, "fibonacci charpoly x coef = -1 = -tr(M)")
    T(cp[3] == -1, "fibonacci charpoly constant = -1 = (-1)^2 * det")
    local inv = sp.spectral.invariants_2x2(fib[1][1], fib[1][2], fib[2][1], fib[2][2])
    local qe = sp.spectral.qbeta_eigenvalue(fib, cp, 40)
    T(qe.converged, "fibonacci qbeta eigenvalue converged")
    NEAR(qe.lambda, inv.beta, 1e-9,
        string.format("fibonacci qbeta lambda (%.6f) matches beta (%.6f)",
            qe.lambda, inv.beta))
end

-- Plastic matrix [[0,0,1],[1,0,1],[0,1,0]]
--   charpoly = x^3 - x - 1
--   In LUA (1-indexed): cp[1]=1, cp[2]=0 (coef of x^2 = -tr = 0),
--   cp[3]=-1 (coef of x = -1), cp[4]=-1 (constant).
local function charpoly_int_plastic()
    local plastic = { {0, 0, 1}, {1, 0, 1}, {0, 1, 0} }
    local cp = sp.spectral.charpoly_int(plastic)
    T(#cp == 4, "plastic charpoly has 4 coefficients")
    T(cp[1] == 1, "plastic charpoly is monic")
    T(cp[2] == 0, "plastic charpoly x^2 coef = 0 = -tr(M)")
    T(cp[3] == -1, "plastic charpoly x coef = -1")
    T(cp[4] == -1, "plastic charpoly constant = -1 = (-1)^3 * det")
    local inv = sp.spectral.invariants_3x3(plastic)
    local qe = sp.spectral.qbeta_eigenvalue(plastic, cp, 60)
    T(qe.converged, "plastic qbeta eigenvalue converged")
    NEAR(qe.lambda, inv.beta, 1e-9,
        string.format("plastic qbeta lambda (%.6f) matches beta (%.6f)",
            qe.lambda, inv.beta))
end

-- 4x4 Tetrabonacci matrix -- the "first n>=4 the legacy 8-ring
-- path couldn't reach" case.  Per DIRECTION thread C item 1,
-- charpoly_int(M) is the entry point that lets the general-d
-- bit-exact path work at any alphabet size.
--   Tetrabonacci matrix [[1,1,1,1],[1,0,0,0],[0,1,0,0],[0,0,1,0]]
--   charpoly = x^4 - x^3 - x^2 - x - 1
--   det(M) = -1 (NOT +1; verified via sympy), so constant = (-1)^4 * (-1) = -1.
--   In LUA (1-indexed): cp[1]=1, cp[2]=-1 (x^3 coef = -tr = -1),
--   cp[3]=-1, cp[4]=-1, cp[5]=-1 (constant).
local function charpoly_int_tetrabonacci()
    local tetra = { {1, 1, 1, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0} }
    local cp = sp.spectral.charpoly_int(tetra)
    T(#cp == 5, string.format("tetrabonacci charpoly has 5 coefficients (got %d)", #cp))
    T(cp[1] == 1, "tetrabonacci charpoly is monic")
    T(cp[2] == -1, "tetrabonacci charpoly x^3 coef = -1 = -tr(M)")
    T(cp[5] == -1, "tetrabonacci charpoly constant = -1 = (-1)^4 * det(M) = det(M) since det = -1")
    -- The 4x4 beta is the tetranacci constant (~1.92756).
    local qe = sp.spectral.qbeta_eigenvalue(tetra, cp, 60)
    T(qe.converged,
        string.format("tetrabonacci qbeta eigenvalue converged (got err=%s)", qe.error))
    -- Known tetranacci beta = 1.9275619754829254...  We compare
    -- against the value recovered by spectral_invariants_general
    -- (double precision) which is good to ~1e-9 for this size.
    NEAR(qe.lambda, 1.9275619754829254, 1e-9,
        string.format("tetrabonacci qbeta lambda (%.10f) matches expected beta",
            qe.lambda))
end

-- ---------- qbeta_eigenvalue: the bit-exact dominant eigenvalue ----------

-- The key claim of this layer: the bit-exact Q(beta) eigenvalue
-- matches the double-precision power-iteration value to high
-- precision.  This is a necessary sanity check that the general
-- layer isn't computing something different from the legacy path.
-- We check on the 11-candidate clean Pisot set (Tetrabonacci +
-- σ₁ + σ₂ + the 8 from rnd1..rnd8, but at minimum the three
-- named ones).
local function qbeta_matches_double_precision_all_named()
    local cases = {
        { name = "tribonacci",  subst = { {0, 1}, {0, 2}, {0} } },
        { name = "plastic",     subst = { {1}, {2}, {0, 1} } },
        { name = "supergolden", subst = { {0, 1, 2}, {0}, {1} } },
    }
    for _, c in ipairs(cases) do
        local m = sp.substitution.matrix(c.subst)
        local cp = sp.spectral.charpoly_int(m)
        local inv = sp.spectral.invariants_3x3(m)
        local qe = sp.spectral.qbeta_eigenvalue(m, cp, 60)
        T(qe.converged,
            string.format("%s: qbeta converged (err=%s)", c.name, qe.error))
        local rel = math.abs(qe.lambda - inv.beta) / inv.beta
        T(rel < 1e-9,
            string.format("%s: |qbeta - beta| / beta (%.2e) < 1e-9",
                c.name, rel))
    end
end

-- 4x4: Tetrabonacci.  This is the case the 8-fixed-ring legacy
-- path COULDN'T REACH (no named ring beyond Tetrabonacci at this
-- alphabet size, and the general-d path was the only way forward).
-- We compare against the closed-form tetranacci value
-- (1.9275619754829254...), not against spectral_invariants_3x3 which
-- only operates on 3x3 sub-dispatch.
local function qbeta_matches_double_precision_tetrabonacci()
    local tetra = { {1, 1, 1, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0} }
    local cp = sp.spectral.charpoly_int(tetra)
    local qe = sp.spectral.qbeta_eigenvalue(tetra, cp, 60)
    T(qe.converged, "tetrabonacci qbeta converged at 4x4 (the n>=4 case the 8-ring path couldn't reach)")
    NEAR(qe.lambda, 1.9275619754829254, 1e-9,
        string.format("tetrabonacci n=4: qbeta (%.10f) matches closed-form tetranacci beta (1.9275619754829254)",
            qe.lambda))
end

local function registry()
    return {
        { name = "charpoly_int_tribonacci",                  fn = charpoly_int_tribonacci },
        { name = "charpoly_int_fibonacci",                   fn = charpoly_int_fibonacci },
        { name = "charpoly_int_plastic",                     fn = charpoly_int_plastic },
        { name = "charpoly_int_tetrabonacci",                fn = charpoly_int_tetrabonacci },
        { name = "qbeta_matches_double_precision_all_named", fn = qbeta_matches_double_precision_all_named },
        { name = "qbeta_matches_double_precision_tetrabonacci", fn = qbeta_matches_double_precision_tetrabonacci },
    }
end

local function main(spectre_module)
    sp = spectre_module
    for _, c in ipairs(registry()) do
        TEST(c.name, c.fn)
    end
    return R.run_all()
end

return { main = main }
