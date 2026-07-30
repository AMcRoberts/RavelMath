-- lua/ravel/tests/test_pisot_survey.lua
--
-- Random Pisot substitution survey + Salem thermodynamics + the
-- Pisot conjecture certificate (certified-PP implies rho_nc < beta).
-- Mirrors the reference `python/pisot_survey.py`, `python/salem_thermo.py`,
-- and the reference `closure_test.py` exactly.  Every call goes
-- through the C++ backend (`ravel.survey.*`); no Python is
-- invoked.
--
-- The Pisot conjecture is a halting-style statement: every
-- irreducible primitive Pisot substitution has pure discrete
-- spectrum, i.e. its balanced-pair BFS terminates AND every
-- irreducible pair reaches a coincidence chunk.  The
-- certification margin log(beta/rho_nc) measures how far the
-- substitution is from the Salem boundary.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local NEAR = runner.assert_near
local T    = runner.assert_true
local F    = runner.assert_false

local sp

-- ---------- Pisot conjecture certificate ----------

-- For a primitive Pisot substitution that is certified PURE
-- DISCRETE, the conjecture requires log(beta / rho_nc) > 0, i.e.
-- rho_nc < beta.  This is the "margin" the reference thermometer
-- measures.  Note that the difficulty driving the random-survey
-- *censoring rate* (when `certified == false` because the BFS
-- hit the cap) is a **combinatorial size effect** (bigger matrix
-- entries -> larger balanced-pair search space), per the reference
-- agent's ANSWERS_TO_QUESTIONS.txt (§3.9); the earlier p-adic
-- / ultrametric framing was refuted twice (discriminant proxy,
-- GF(p) kernel dim of M).  Salems at the wall remain hard because
-- of matrix-entry magnitude, not number-theoretic obstruction.
local function pisot_conjecture_margin_tribonacci()
    local trib = { {0, 1}, {0, 2}, {0} }
    local m = sp.substitution.matrix(trib)
    local inv = sp.spectral.invariants_3x3(m)
    local rho = sp.balanced_pair.rho_nc(trib)
    T(rho > 0 and rho < inv.beta,
        string.format("tribonacci rho_nc (%.6f) < beta (%.6f)",
            rho, inv.beta))
end

local function pisot_conjecture_margin_plastic()
    local plastic = { {1}, {2}, {0, 1} }  -- same as Lua's "b","c","ab"
    local m = sp.substitution.matrix(plastic)
    local inv = sp.spectral.invariants_3x3(m)
    local rho = sp.balanced_pair.rho_nc(plastic)
    T(rho > 0 and rho < inv.beta,
        string.format("plastic rho_nc (%.6f) < beta (%.6f)",
            rho, inv.beta))
end

-- ---------- classify_matrix ----------

-- All primitive Pisot substitutions in the survey must satisfy
-- classify_matrix = { pisot=true, irred=true }.
local function classify_known_pisot()
    -- Tribonacci matrix [[1,1,1],[1,0,0],[0,1,0]]
    local trib = { {1, 1, 1}, {1, 0, 0}, {0, 1, 0} }
    local c = sp.survey.classify_matrix(trib)
    T(c.pisot, "tribonacci matrix is Pisot")
    T(c.irred, "tribonacci matrix is irreducible over Q")
    NEAR(c.beta, 1.839286755214161, 1e-9, "tribonacci beta")

    -- Plastic matrix [[0,0,1],[1,0,1],[0,1,0]]
    local plastic = { {0, 0, 1}, {1, 0, 1}, {0, 1, 0} }
    local cp = sp.survey.classify_matrix(plastic)
    T(cp.pisot, "plastic matrix is Pisot")
    T(cp.irred, "plastic matrix is irreducible over Q")
    NEAR(cp.beta, 1.324717957244746, 1e-9, "plastic beta")
end

-- Non-Pisot control: Thue-Morse [[0,1],[1,0]] has all other
-- conjugates on the unit circle.  classify_matrix must say
-- pisot = false.
local function classify_non_pisot()
    local tm = { {0, 1}, {1, 0} }
    local c = sp.survey.classify_matrix(tm)
    F(c.pisot, "Thue-Morse matrix is NOT Pisot (|b2| = 1.0)")
end

-- ---------- pf_right (frequencies) ----------

-- Right Perron eigenvector of the Fibonacci matrix.
-- fibonacci: a->ab, b->a.  M = [[1,1],[1,0]].
-- Column lengths are (2, 1), total 3; normalised frequencies are
-- (2/3, 1/3), but the right Perron eigenvector is (1/phi^2, 1/phi^3)
-- normalised to sum 1, i.e. approximately (0.618, 0.382).
-- The discrepancy is because the right Perron eigenvector is
-- weighted by the relative frequencies of letters, not by the
-- column sums; in the 2x2 case these agree, but in n>2 they
-- differ.  For Fibonacci, M is symmetric, so left and right
-- Perron eigenvectors coincide, and the right one is
-- (1/phi^2, 1/phi^3) / (1/phi^2 + 1/phi^3) = (1/phi^2, 1/phi^3)/1.
local function pf_right_fibonacci()
    local fib = { {1, 1}, {1, 0} }
    local f = sp.survey.pf_right(fib)
    -- Note: the convention used by pf_right is the LAST-dimension
    -- component of the right Perron eigenvector, normalised to
    -- sum 1.  For Fibonacci matrix [[1,1],[1,0]] this is
    -- (~0.618, ~0.382).
    NEAR(f[1], 0.61803398875, 1e-6, "Perron freq(a) for Fibonacci")
    NEAR(f[2], 0.38196601125, 1e-6, "Perron freq(b) for Fibonacci")
    NEAR(f[1] + f[2], 1.0, 1e-12, "frequencies sum to 1")
end

-- ---------- ceiling_running ----------

-- The running ceiling of the worst Dirichlet return to 0 or 1/2.
-- Mirror python/pisot_survey.py::ceiling_running at small L.
local function ceiling_running_fibonacci()
    local fib = { {1, 1}, {1, 0} }
    -- Expand substitution to N=2^14 first via the C++ thermometer.
    local sigma = { {0, 1}, {0} }
    local word = sp.substitution.expand(sigma, 0, 1 << 14)
    -- Estimate frequencies from the empirical word distribution.
    local counts = { 0, 0 }
    for _, c in ipairs(word) do
        if c == 0 then counts[1] = counts[1] + 1 end
        if c == 1 then counts[2] = counts[2] + 1 end
    end
    local f = { counts[1] / #word, counts[2] / #word }
    local ceiling = sp.survey.ceiling_running(f, 1024, 33)
    T(ceiling[33] > 0, "ceiling at L=33 is positive")
    -- The ceiling is non-increasing on the running-minimum pass.
    local prev = math.huge
    for i = 33, #ceiling do
        T(ceiling[i] <= prev + 1e-12,
            string.format("ceiling[%d]=%.6g <= ceiling[%d]=%.6g",
                i, ceiling[i], i-1, prev))
        prev = ceiling[i]
    end
end

-- ---------- Random Pisot survey ----------

-- A small survey: generate a few random primitive Pisot
-- substitutions, certify each, compute rho_nc where
-- certification succeeds.  This reproduces the body of
-- python/pisot_survey.py::run without numpy or scipy.
local function small_random_survey()
    local instances = sp.survey.random(8, 11, 8000, 60000)
    T(#instances >= 5,
        string.format("survey yielded at least 5 Pisot instances (got %d)",
            #instances))
    for _, inst in ipairs(instances) do
        -- Every instance must be Pisot and irreducible.
        T(inst.pisot,
            string.format("random %s: pisot=true", inst.name))
        T(inst.beta > 1.0,
            string.format("random %s: beta > 1.0 (got %.4f)",
                inst.name, inst.beta))
        -- Certify -> conjecture: if certified, must have
        -- rho_nc < beta (instance-wise Pisot margin is strict).
        if inst.certified then
            T(inst.rho_nc > 0 and inst.rho_nc < inst.beta,
                string.format("random %s: certified -> rho_nc (%.6f) < beta (%.6f)",
                    inst.name, inst.rho_nc, inst.beta))
        end
    end
end

-- ---------- Public Pisot conjecture marker ----------

-- Conjecture: every irreducible primitive Pisot substitution
-- has certified == true and rho_nc < beta.  The reference
-- survey (1238 boundary-biased instances, plus the 38 in
-- pisot_survey.jsonl) has never observed a counterexample.
local function conjecture_certificate_passed_in_menu()
    local trib = { {0, 1}, {0, 2}, {0} }
    local sp_data = subst_matrix_to_objs(trib)
    local cert = sp.balanced_pair.certify(trib, 8000, 60000)
    T(cert.terminated and cert.certified,
        "tribonacci is certified PURE DISCRETE")
end

-- ---------- Random Pisot survey at fixed alphabet sizes ----------
--
-- The general-n path: `survey.random(target, seed, max_pairs, max_len,
-- alphabet_size)` should produce Pisot-candidates and certify the
-- ones whose balanced-pair BFS terminates within caps.  Pin
-- alphabet sizes 4 and 5 so a future regression in the spectral
-- classifier or the random-matrix generator can't silently drop
-- these cases.
local function survey_generalizes_past_n3()
    local n4 = sp.survey.random(2, 7, 8000, 60000, 4)
    T(#n4 >= 1,
        string.format("n=4 survey yields at least 1 Pisot candidate (got %d)", #n4))
    for _, inst in ipairs(n4) do
        T(inst.beta > 1.0,
            string.format("%s: beta > 1.0 (got %.4f)", inst.name, inst.beta))
        T(inst.pisot,
            string.format("%s: pisot=true", inst.name))
        if inst.certified then
            T(inst.rho_nc > 0 and inst.rho_nc < inst.beta,
                string.format("%s: certified -> rho_nc (%.6f) < beta (%.6f)",
                    inst.name, inst.rho_nc, inst.beta))
        end
    end

    -- n=5 is harder (smaller Pisot density at this alphabet size
    -- with the default K range); we don't require a hit, just
    -- that the survey generator returns cleanly without
    -- crashing.
    local n5 = sp.survey.random(1, 7, 8000, 60000, 5)
    for _, inst in ipairs(n5) do
        T(inst.beta > 1.0,
            string.format("%s: beta > 1.0 (got %.4f)", inst.name, inst.beta))
    end
end

-- Reference `python/pisot_survey.py` generated rnd1_canon as one of
-- the 24 random Pisot substitution survey rows in
-- pisot_survey.jsonl.  This is one of the **|det|=2** Pisot
-- certified-PP entries (ANSWERS §3.8: "plenty of |det|=2 rows,
-- NO |det|=3 rows").  This test pins the local C++ reproduction
-- of the reference JSONL row.
local function rnd1_canon_det2_pisot()
    -- From pisot_survey.jsonl row "rnd1_canon":
    -- subst = {0:[1], 1:[0,2,2], 2:[0,0,1,2,2]}
    local subst = { {1}, {0, 2, 2}, {0, 0, 1, 2, 2} }
    local m = sp.substitution.matrix(subst)
    local inv = sp.spectral.invariants_3x3(m)
    NEAR(inv.beta_abs, 3.152757602010394, 1e-9,
        "rnd1_canon beta (reference JSONL) matches bit-exact")
    -- det M = 2.
    NEAR(math.abs(inv.det_M), 2, 1e-9,
        "rnd1_canon |det M| = 2 (non-unimodular Pisot)")
    -- certify with reference caps (8000 pairs / 60000 length)
    local cert = sp.balanced_pair.certify(subst, 8000, 60000)
    T(cert.terminated,
        "rnd1_canon BFS terminated within reference caps")
    T(cert.certified,
        "rnd1_canon certified PURE DISCRETE (per ABBLS Thm 5.3)")
    NEAR(cert.n_irreducible, 131, 1,
        string.format("rnd1_canon n_irreducible = 131 (got %d)",
            cert.n_irreducible))
    local rho = sp.balanced_pair.rho_nc(subst, 8000, 60000)
    T(rho > 0 and rho < inv.beta_abs,
        string.format("rnd1_canon rho_nc (%.4f) < beta (%.4f) [non-unimodular Pisot survives conjecture]",
            rho, inv.beta_abs))
end

-- ---------- rnd24_canon: substantively near-Salem ----------

-- Per ANSWERS §3.3: subst = {0:[2], 1:[0,1,1], 2:[1,1,2]} is
-- certified = True, n_irr = 586, rho_nc = 2.4765733851476837,
-- beta = 2.521379706804568, ratio = 0.9822 (CLOSE to beta but
-- not exactly equal).  Verdict: substantive (a real
-- near-Salem-boundary instance), not degenerate.
local function rnd24_canon_near_salem()
    local subst = { {2}, {0, 1, 1}, {1, 1, 2} }
    local m = sp.substitution.matrix(subst)
    local inv = sp.spectral.invariants_3x3(m)
    NEAR(inv.beta_abs, 2.521379706804568, 1e-9,
        "rnd24_canon beta (reference) bit-exact")
    NEAR(inv.beta2, 0.890627029385581, 1e-9,
        "rnd24_canon |b2| (reference) bit-exact (high |b2| ~ 0.89 marks Salem-proximity)")
    local cert = sp.balanced_pair.certify(subst, 8000, 60000)
    T(cert.terminated,
        "rnd24_canon BFS terminated within reference caps (586 pairs)")
    T(cert.certified,
        "rnd24_canon certified PURE DISCRETE (a Pisot near the Salem wall)")
    NEAR(cert.n_irreducible, 586, 1,
        string.format("rnd24_canon n_irreducible = 586 (got %d)",
            cert.n_irreducible))
    local rho = sp.balanced_pair.rho_nc(subst, 8000, 60000)
    NEAR(rho, 2.4765733851476837, 1e-9,
        string.format("rnd24_canon rho_nc = 2.4766... (got %.9f)", rho))
    local ratio = rho / inv.beta_abs
    NEAR(ratio, 0.982229, 1e-4,
        string.format("rnd24_canon rho/beta = 0.982 (substantive near-degenerate but NOT exactly 1.0; got %.6f)",
            ratio))
end

-- Helper that converts a 1-indexed subst table to a 0-indexed
-- list-of-lists for the C++ matrix helper.
function subst_matrix_to_objs(sigma)
    -- Used only to confirm by the conjecture marker test
    -- above; the actual matrix comes from sp.substitution.matrix.
    return sigma
end

local function registry()
    return {
        { name = "pisot_conjecture_margin_tribonacci",   fn = pisot_conjecture_margin_tribonacci },
        { name = "pisot_conjecture_margin_plastic",      fn = pisot_conjecture_margin_plastic },
        { name = "classify_known_pisot",                fn = classify_known_pisot },
        { name = "classify_non_pisot",                  fn = classify_non_pisot },
        { name = "pf_right_fibonacci",                   fn = pf_right_fibonacci },
        { name = "ceiling_running_fibonacci",            fn = ceiling_running_fibonacci },
        { name = "small_random_survey",                 fn = small_random_survey },
        { name = "survey_generalizes_past_n3",          fn = survey_generalizes_past_n3 },
        { name = "conjecture_certificate_passed_in_menu", fn = conjecture_certificate_passed_in_menu },
        { name = "rnd1_canon_det2_pisot",               fn = rnd1_canon_det2_pisot },
        { name = "rnd24_canon_near_salem",               fn = rnd24_canon_near_salem },
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
-- (registry entries appended below)
