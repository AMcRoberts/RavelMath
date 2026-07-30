-- lua/tests/test_closure.lua
--
-- The CORRECTED closure law for the Tribonacci noncoincidence rate.
--
-- The naive boundary-growth hypothesis (used in the C++ Tribonacci
-- test and `docs/THEOREM_STATUS.md` section 3) is
--
--     rho_nc = beta * |alpha|^(2-s)
--
-- which for the Tribonacci Pisot constant fits the empirical
-- noncoincidence growth rate.
--
-- The reference audit (NOTES_PISOT.md [12]; see also
-- ../python/closure_test.py and ../python/refit_wide.py) shows
-- the corrected form
--
--     delta = log(beta/rho_nc) / max(log(1/|b2|), log(beta)/2)
--
-- where delta is the empirical spectral-type exponent observed by
-- the Pisot thermometer (Theta(L) ~ 2 - L^(-delta)).
--
-- Key structural facts verified by this test:
--
--   1. For UNIMODULAR cubic Pisot numbers (|det M| = 1,
--      |b2| = beta^(-1/2)), the two candidate denominators
--      coincide, so the corrected and naive laws agree.  Tribonacci
--      AND plastic are both unimodular cubic Pisot numbers with
--      complex conjugate pair, so they fall in this regime.
--
--   2. For NON-unimodular cubic Pisot (|det M| > 1) with complex
--      pair, |b2| = sqrt(|det|/beta) > beta^(-1/2), so log(1/|b2|)
--      is SMALLER than log(beta)/2 and the max picks log(beta)/2.
--      The corrected and naive laws then BOTH reduce to
--      delta = 2 log(beta/rho_nc)/log(beta); see the synthetic
--      det=2 matrix below.
--
--   3. The Pisot substitution conjecture, in this language, is
--      that delta > 0 instance-wise for every irreducible Pisot
--      substitution, i.e. rho_nc < beta.  The reference survey
--      (1238 boundary-biased instances) finds no certified counter
--      example; the margin log(beta/rho_nc) is empirically shrink-
--      ing toward the Salem wall but never reaches zero.
--
-- This test is a CLOSED-FORM arithmetic check, NOT a simulation.
-- It uses the spectral invariants helper from the C++ backend.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local NEAR = runner.assert_near
local T = runner.assert_true

local sp

-- Tribonacci substitution incidence matrix M:
--   sigma(a) = ab, sigma(b) = ac, sigma(c) = a
--   so M[r][c] = count of r in sigma(c):
--     M[0][0]=1 (a from a->ab), M[1][0]=1 (b from a->ab)
--     M[0][1]=1 (a from b->ac), M[2][1]=1 (c from b->ac)
--     M[0][2]=1 (a from c->a)
-- char poly: x^3 - x^2 - x - 1 (Pisot, beta ≈ 1.839286755...).
local function tribonacci_constants()
    local M = { {1, 1, 1}, {1, 0, 0}, {0, 1, 0} }
    local inv = sp.spectral.invariants_3x3(M)
    return {
        M         = M,
        beta      = inv.beta,
        beta_abs  = inv.beta_abs,
        beta2     = inv.beta2,
        det_M     = inv.det_M,
        bound_rhs = inv.bound_rhs,
    }
end

-- Plastic Pisot substitution (smallest Pisot number, real root of
-- x^3 = x + 1):
--   sigma(a) = b, sigma(b) = c, sigma(c) = ca
--   incidence matrix (column = image):
--     col 0 (image of a, "b"):   M[1][0] += 1
--     col 1 (image of b, "c"):   M[2][1] += 1
--     col 2 (image of c, "ca"):  M[0][2] += 1, M[1][2] += 1
-- char poly: x^3 - x - 1 (Pisot, beta ≈ 1.324717957).
local function plastic_constants()
    local M = { {0, 0, 1}, {1, 0, 1}, {0, 1, 0} }
    local inv = sp.spectral.invariants_3x3(M)
    return {
        M         = M,
        beta      = inv.beta,
        beta_abs  = inv.beta_abs,
        beta2     = inv.beta2,
        det_M     = inv.det_M,
        bound_rhs = inv.bound_rhs,
    }
end

-- The identity |b2|^2 * beta = |det M|, used to verify both
-- Tribonacci and plastic Pisot eigenvalues lie on the right curve.
local function verify_beta2_squared(name, c)
    local lhs = c.beta2 * c.beta2
    local rhs = math.abs(c.det_M) / c.beta_abs
    NEAR(lhs, rhs, 1e-12,
        string.format("%s: |b2|^2 = |det|/|beta| identity", name))
end

-- The Tribonacci case: both candidate denominators are equal, so
-- the naive boundary-growth relation rho_nc = beta*alpha^(2-s) at
-- s = 1.093364 (root of 2 alpha^(3s) + alpha^(4s) = 1) gives the
-- same delta as the corrected closure form delta = 2 log(beta/rho_nc)/log(beta).
local function tribonacci_corrected_equals_naive()
    local c = tribonacci_constants()
    verify_beta2_squared("Tribonacci", c)
    T(math.abs(c.det_M) == 1,
        "Tribonacci matrix is unimodular (|det|=1)")
    local log_inv_b2    = math.log(1.0 / c.beta2)
    local half_log_beta = 0.5 * math.log(c.beta)
    NEAR(log_inv_b2, half_log_beta, 1e-12,
        "Tribonacci: log(1/|b2|) == log(beta)/2 (unimodular cubic)")
    -- The naive rho_nc at s = 1.093364:
    local alpha = math.sqrt(1.0 / c.beta)
    local s = 1.093364164
    local required_rho_nc = c.beta * alpha ^ (2.0 - s)
    NEAR(required_rho_nc, 1.395337, 1e-4,
        "Tribonacci naive-law rho_nc at s=1.093364 is ~1.395337")
    -- Closed-form dimension check: 2 alpha^(3s) + alpha^(4s) = 1.
    local lhs = 2.0 * alpha ^ (3.0 * s) + alpha ^ (4.0 * s)
    NEAR(lhs, 1.0, 1e-6,
        "Tribonacci boundary equation 2 alpha^(3s) + alpha^(4s) = 1 at s=1.093364")
    -- The corrected closure law evaluated at this rho_nc gives delta = 2 - s.
    local delta_corrected = math.log(c.beta / required_rho_nc) / math.max(log_inv_b2, half_log_beta)
    NEAR(delta_corrected, 2.0 - s, 1e-6,
        "Tribonacci corrected closure delta == 2 - s for unimodular Pisot")
end

local function plastic_corrected_equals_naive()
    local c = plastic_constants()
    verify_beta2_squared("Plastic", c)
    T(math.abs(c.det_M) == 1,
        "Plastic matrix is unimodular (|det|=1)")
    T(c.beta > 1.32 and c.beta < 1.33,
        "Plastic beta ~ 1.3247 (smallest Pisot number)")
    local log_inv_b2    = math.log(1.0 / c.beta2)
    local half_log_beta = 0.5 * math.log(c.beta)
    NEAR(log_inv_b2, half_log_beta, 1e-9,
        "Plastic: log(1/|b2|) == log(beta)/2 (unimodular cubic)")
end

-- For non-unimodular cubic Pisot with complex pair, |b2| > beta^(-1/2)
-- makes log(1/|b2|) SMALLER than log(beta)/2, so the max picks
-- log(beta)/2 (the larger).  The corrected law then equals
-- delta = 2 log(beta/rho_nc)/log(beta).  This is a STRUCTURAL test
-- on the spectral invariants only; the synthetic matrix is not
-- necessarily Pisot, but |det|=2 makes the inequality direction
-- unambiguous.
local function non_unimodular_correction_is_inactive()
    local M = { {2, 1, 1}, {1, 1, 0}, {0, 1, 1} }
    local inv = sp.spectral.invariants_3x3(M)
    NEAR(math.abs(inv.det_M), 2, 1e-9, "synthetic |det| = 2")
    local lhs = inv.beta2
    local rhs = math.sqrt(2.0 / inv.beta_abs)
    NEAR(lhs, rhs, 1e-9, "|b2| = sqrt(|det|/|beta|) for det=2")
    T(lhs > math.pow(inv.beta_abs, -0.5),
        "|b2| > beta^(-1/2) when |det|=2: corrected denominator = log(beta)/2")
    local log_inv_b2    = math.log(1.0 / inv.beta2)
    local half_log_beta = 0.5 * math.log(inv.beta_abs)
    T(log_inv_b2 < half_log_beta,
        "non-unimodular: log(1/|b2|) < log(beta)/2 (max picks half_log_beta)")
end

local function registry()
    return {
        { name = "closure_tribonacci_corrected_equals_naive",
          fn   = tribonacci_corrected_equals_naive },
        { name = "closure_plastic_corrected_equals_naive",
          fn   = plastic_corrected_equals_naive },
        { name = "closure_non_unimodular_correction_is_inactive",
          fn   = non_unimodular_correction_is_inactive },
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
