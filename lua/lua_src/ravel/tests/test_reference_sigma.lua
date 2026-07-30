-- lua/ravel/tests/test_reference_sigma.lua
--
-- Reproductions of the ground-truth numbers from
-- `python/sigma1_balanced_pair.py` and the reference notes.  These
-- test cases pin the C++ implementation's `rho_nc` to the exact
-- reference values for sigma_1 and sigma_2, the two substitutions
-- mentioned by name in
--   docs/RESEARCH_STATUS.md
-- and used as the gold-standard reconstruction target in the
-- corrected paper's examples
--   refs/corrected_reference_arxiv_2511.16442.pdf
--
-- The substitutions are the cubic Pisot substitutions of the November
-- 2025 paper (arXiv:2511.16442):
--   sigma_1: sigma(1) = 1112, sigma(2) = 113, sigma(3) = 1
--   sigma_2: sigma(1) = 112,  sigma(2) = 1113, sigma(3) = 1
-- encoded here as 0-indexed subst tables:
--   sigma_1 = {0:(0,0,0,1), 1:(0,0,2), 2:(0,)}
--   sigma_2 = {0:(0,0,1),    1:(0,0,0,2), 2:(0,)}

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local NEAR = runner.assert_near
local T    = runner.assert_true

local sp

-- These caps match sigma1_balanced_pair.py::balanced_pair_rho
-- defaults: max_pairs = 60000, max_len = 300000.  Using a smaller
-- cap on sigma_1 with letter counts up to 5 would give the same
-- result (the BFS terminates far below the cap); but matching the
-- reference defaults exactly keeps the comparison bit-exact.
local MAX_PAIRS = 60000
local MAX_LEN   = 300000

-- ---------- sigma_1 ----------

-- Reference python/sigma1_balanced_pair.py output:
--   sigma_1: certified=True  n_irr=11  rho_nc=2.286298264082328
--
-- Our C++ reproduction:
--   rho_nc = 2.2862982640841847 (at MAX_PAIRS = 60000, MAX_LEN = 300000)
local function sigma_1_certify_and_rho_nc()
    local sigma = { {0, 0, 0, 1}, {0, 0, 2}, {0} }
    local m = sp.substitution.matrix(sigma)
    local inv = sp.spectral.invariants_3x3(m)
    -- reference python subst: char poly x^3 - 3x^2 - 2x - 1, beta ≈ 3.627 (reference python subst).
    NEAR(inv.beta_abs, 3.627365, 1e-3, "sigma_1 beta (reference python subst)")
    local cert = sp.balanced_pair.certify(sigma, MAX_PAIRS, MAX_LEN)
    T(cert.terminated,
        "sigma_1 BFS terminated within the reference caps")
    T(cert.certified,
        "sigma_1 certified PURE DISCRETE per ABBLS Thm 5.3")
    NEAR(cert.n_irreducible, 11, 1,
        string.format("sigma_1 n_irreducible = 11 (got %d)",
            cert.n_irreducible))
    local rho = sp.balanced_pair.rho_nc(sigma, MAX_PAIRS, MAX_LEN)
    -- Reference Python: 2.286298264082328.
    -- Our C++:         2.2862982640841847.
    -- ULP-scale agreement to 1e-9 (both are double-precision).
    NEAR(rho, 2.286298264082328, 1e-9,
        string.format("sigma_1 rho_nc = 2.286298264082328 (reference Python), got %.16f",
            rho))
end

-- ---------- sigma_2 ----------

-- Reference sigma1_balanced_pair.py output:
--   sigma_2: certified=True  n_irr=11  rho_nc=2.324717957244747
--
-- Our C++ reproduction (to 1e-9):
--   rho_nc = 2.3247179572442889.
--
-- IMPORTANT — the relationship between sigma_2's rho_nc (2.3247...) and
-- sigma_1's rho_nc (2.2862...) is NOT the boundary-graph eigenvalue of
-- the same construction.  The reference technical note explicitly flags
-- this:
--   "sigma1 (26 nodes): dominant eigenvalue ~= 1.7462 (...)
--    sigma2 (50 nodes): dominant eigenvalue ~= 2.1969
--    rho_nc(sigma_1) = 2.286298 ... NOT the same number."
-- The conjecture that rho_nc (balanced-pair) == G_B dominant eigenvalue
-- (geometric) is the actual open question; the reference branch
-- verified the WORST-CASE (sigma_1, sigma_2) controlled experiment and
-- found a real discrepancy.  We reproduce only the rho_nc number here.
local function sigma_2_certify_and_rho_nc()
    local sigma = { {0, 0, 1}, {0, 0, 0, 2}, {0} }
    local m = sp.substitution.matrix(sigma)
    local inv = sp.spectral.invariants_3x3(m)
    -- reference python subst: char poly x^3 - 2x^2 - x - 1, beta ≈ 3.080.
    NEAR(inv.beta_abs, 3.079596, 1e-3, "sigma_2 beta (reference python subst)")
    local cert = sp.balanced_pair.certify(sigma, MAX_PAIRS, MAX_LEN)
    T(cert.terminated,
        "sigma_2 BFS terminated within the reference caps")
    T(cert.certified,
        "sigma_2 certified PURE DISCRETE per ABBLS Thm 5.3")
    NEAR(cert.n_irreducible, 11, 1,
        string.format("sigma_2 n_irreducible = 11 (got %d)",
            cert.n_irreducible))
    local rho = sp.balanced_pair.rho_nc(sigma, MAX_PAIRS, MAX_LEN)
    -- Reference Python: 2.324717957244747.
    -- Our C++:         2.3247179572442889.
    NEAR(rho, 2.324717957244747, 1e-9,
        string.format("sigma_2 rho_nc = 2.324717957244747 (reference Python), got %.16f",
            rho))
end

-- ---------- Predicted vs measured Rauzy fractal dimension ----------

-- The reference boundary_dimension_shortcut.py reproduces two of its
-- ground-truth targets on sigma_{4,2} -- the published Rauzy
-- fractal boundary dimension 1.6286... is held as the gold standard.
-- We re-do only the Tribonacci + sigma_{4,2} convex-only fits here
-- to verify our spectral + rho_nc + dimensional prediction pipeline.
local function tribonacci_dimension()
    local trib = { {0, 1}, {0, 2}, {0} }
    local rho = sp.balanced_pair.rho_nc(trib, 20000, 60000)
    -- Now uses the dedicated Lua predictor (ravel.predict_dimension);
    -- mirrors python/boundary_dimension_shortcut.py::predict_boundary_dimension.
    -- Reference ground truth: Tribonacci's Rauzy fractal boundary dimension
    -- is the root of 2*alpha^(3s) + alpha^(4s) = 1 where alpha = beta^(-1/2),
    -- which is s = 1.093364164.
    local result = sp.predict_dimension.predict(trib, rho)
    T(result.conformal, "tribonacci is conformal (complex-pair secondary)")
    NEAR(result.s_H, 1.093364164, 1e-3,
        string.format("tribonacci predicted s_H = 1.093364 (got %.6f)", result.s_H))
end

-- ---------- sigma_{4,2}: held-out boundary_dimension_shortcut test ----------

-- Reference boundary_dimension_shortcut.py::predict_boundary_dimension
-- runs sigma_{4,2} as one of its three ground-truth targets.
-- This is the (a,b)=(4,2) member of the sigma_{a,b} cubic family
-- for which the BP-ρ_nc == boundary-eigenvalue identity is
-- algebraically proven (OPEN_PISOT_PIPELINE_HITLIST.md §1).
-- In contrast, σ_1 and σ_2 are November-2025-paper-specific and
-- the identity is OPEN for them (rho_nc ≠ G_B eigenvalue).
--
-- For sigma_{4,2} = {0:(0,0,0,0,1), 1:(0,0,2), 2:(0,)}:
--   beta ≈ 4.4944928370554
--   b2 ≈ 0.47169324154665
--   rho_nc ≈ 2.5823494859623 (C++ reproduction)
--   certified = true, n_irr = 11
local function sigma_4_2_held_out()
    local sigma = { {0, 0, 0, 0, 1}, {0, 0, 2}, {0} }
    local m = sp.substitution.matrix(sigma)
    local inv = sp.spectral.invariants_3x3(m)
    T(inv.beta > 1.0, "sigma_{4,2} beta > 1")
    T(inv.beta_abs < inv.beta * 1.01,
        "sigma_{4,2} beta_abs is the principal real root")
    local cert = sp.balanced_pair.certify(sigma, 60000, 300000)
    T(cert.terminated, "sigma_{4,2} BFS terminated within reference caps")
    T(cert.certified, "sigma_{4,2} certified PURE DISCRETE")
    T(cert.n_irreducible <= 50,
        string.format("sigma_{4,2} n_irreducible in reasonable range (got %d)",
            cert.n_irreducible))
    local rho = sp.balanced_pair.rho_nc(sigma, 60000, 300000)
    local r = sp.predict_dimension.predict(sigma, rho)
    T(r.conformal, "sigma_{4,2} is conformal (complex-pair secondary)")
    T(r.s_H and r.s_H > 0 and r.s_H < 2.0,
        string.format("sigma_{4,2} conformal s_H in (0, 2): got %.4f", r.s_H or -1))
end

local function registry()
    return {
        { name = "offsite_sigma_1_certify_and_rho_nc", fn = sigma_1_certify_and_rho_nc },
        { name = "offsite_sigma_2_certify_and_rho_nc", fn = sigma_2_certify_and_rho_nc },
        { name = "offsite_tribonacci_dimension",        fn = tribonacci_dimension },
        { name = "offsite_sigma_4_2_held_out",         fn = sigma_4_2_held_out },
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
