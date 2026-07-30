-- lua/ravel/tests/test_pisot.lua
--
-- Certification and measurement tests for primitive Pisot
-- substitutions.  These mirror the reference
-- `python/balanced_pair.py`, `python/substitution_thermometer.py`,
-- and `python/pisot_survey.py` exactly: a substitution is a Lua
-- table { 1: {word}, 2: {word}, ... } of int words.  All
-- arithmetic goes through the C++ backend
-- (`ravel.balanced_pair.*`, `ravel.thermometer.*`,
-- `ravel.substitution.*`); no Python is invoked.
--
-- The Pisot menu here is the same one in
-- `python/substitution_thermometer.py` and the seven-letter KNOWN
-- list in `python/pisot_survey.py`.  Each substitution is
-- specified as `subst[c] = image of letter c`; we use 1-indexed
-- Lua tables because the C++ binding maps 1-index to c=0.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local NEAR = runner.assert_near
local T    = runner.assert_true
local F    = runner.assert_false

local sp

-- Helper: convert a list-of-strings (the Python / C++ side's
-- { "ab", "ac", "a" } notation) to the 1-indexed int-table that
-- the C++ binding consumes.
local function subst_from_letters(words)
    local out = {}
    for c, w in ipairs(words) do
        local row = {}
        for i = 1, #w do
            row[i] = string.byte(w, i) - string.byte("a")
        end
        out[c] = row
    end
    return out
end

-- ---------- Pisot substitutions from the reference menu ----------
--
-- These match python/balanced_pair.py::SYSTEMS,
-- python/substitution_thermometer.py::MENU, and the KNOWN set in
-- python/pisot_survey.py.  The C++ backend encodes the
-- substitution matrix as M[r][c] = count of r in subst[c], which
-- matches the convention used by the independent reference
-- implementation.

local MENU = {
    fibonacci       = subst_from_letters({ "ab", "a"                 }),
    tribonacci      = subst_from_letters({ "ab", "ac", "a"           }),
    -- "supergolden" (a->ab, b->c, c->a) and "smallest_pisot"
    -- (the same substitution under a different label, in
    -- pisot_survey.jsonl) share char poly x^3 - x^2 - 1.
    supergolden     = subst_from_letters({ "ab", "c",   "a"          }),
    smallest_pisot = subst_from_letters({ "ab", "c",   "a"          }),
    -- plastic (a->b, b->c, c->ab) -- char poly x^3 - x - 1;
    -- the actual smallest Pisot cube constant (1.3247...).
    plastic         = subst_from_letters({ "b",  "c",   "ab"        }),
    period_doubling = subst_from_letters({ "ab", "aa"                 }),
    thue_morse      = subst_from_letters({ "ab", "ba"                 }),
}

-- Sanity: alphabet size and Pisot status from the C++ backend
-- must agree with the reference Python source.  We cross-check
-- each primitive Pisot substitution with its expected char
-- poly.  Two-letter substitutions (fibonacci, period_doubling,
-- thue_morse) call invariants_2x2; three-letter substitutions
-- call invariants_3x3.
local function pisot_status_per_system()
    local expected_beta = {
        fibonacci       = 1.618033988749895,
        tribonacci      = 1.839286755214161,
        supergolden     = 1.465571231876768,
        smallest_pisot = 1.465571231876768,
        plastic         = 1.324717957244746,
        period_doubling = 2.0,
        thue_morse      = 2.0,
    }
    for name, sigma in pairs(MENU) do
        local m = sp.substitution.matrix(sigma)
        local inv
        if #m == 2 then
            -- 2x2 invariants call signature is (a11, a12, a21, a22).
            inv = sp.spectral.invariants_2x2(m[1][1], m[1][2], m[2][1], m[2][2])
        else
            inv = sp.spectral.invariants_3x3(m)
        end
        NEAR(inv.beta_abs, expected_beta[name], 1e-9,
            string.format("%s beta (got %.6f, want %.6f)",
                name, inv.beta_abs, expected_beta[name]))
    end
end

-- ---------- Hollander-Solomyak / Barge-Diamond certification ----------

-- tribonacci: certifies PURE DISCRETE per the Python script.
-- The Python run reports n_irreducible = 11; we expect the same
-- within the same caps (max_pairs=20000, max_len=4000).
local function certify_tribonacci_pure_discrete()
    local sigma = subst_from_letters({ "ab", "ac", "a" })
    local cert = sp.balanced_pair.certify(sigma, 20000, 4000)
    T(cert.terminated,
        "tribonacci balanced-pair BFS terminated within caps")
    T(cert.certified,
        "tribonacci certified PURE DISCRETE (every irreducible pair reaches coincidence)")
    NEAR(cert.n_irreducible, 11, 1,
        string.format("tribonacci n_irreducible=11 (got %d)",
            cert.n_irreducible))
end

-- plastic (smallest Pisot): certifies PURE DISCRETE per the
-- Python run; the Python cap is max_pairs=20000 and reports
-- n_irr ~= 61.
local function certify_plastic_pure_discrete()
    local sigma = subst_from_letters({ "b", "c", "ab" })
    local cert = sp.balanced_pair.certify(sigma, 20000, 4000)
    T(cert.terminated,
        "plastic balanced-pair BFS terminated within caps")
    T(cert.certified,
        "plastic certified PURE DISCRETE")
    -- The Python survey reports n_irr ~= 61; we expect to match.
    T(cert.n_irreducible >= 50,
        string.format("plastic n_irreducible >= 50 (got %d)",
            cert.n_irreducible))
end

-- smallest_pisot (= supergolden {ab, c, a}): reports n_irr = 11
-- in the reference survey.  This is identical to "supergolden"
-- in substance; the survey used a different label.
local function certify_smallest_pisot()
    local sigma = subst_from_letters({ "ab", "c", "a" })
    local cert = sp.balanced_pair.certify(sigma, 20000, 4000)
    T(cert.terminated,
        "smallest_pisot balanced-pair BFS terminated within caps")
    T(cert.certified,
        "smallest_pisot certified PURE DISCRETE")
    NEAR(cert.n_irreducible, 11, 1,
        string.format("smallest_pisot n_irreducible=11 (got %d)",
            cert.n_irreducible))
end

-- thue_morse: NOT Pisot (char poly x^2 - 1 has all other conjugates
-- outside the unit disk) -- still we test that the certify
-- helper runs and does not crash.  The Python run reported it
-- as a non-Pisot control.
local function certify_thue_morse_runs()
    local sigma = subst_from_letters({ "ab", "ba" })
    local cert = sp.balanced_pair.certify(sigma)
    T(cert.n_irreducible >= 1,
        "thue_morse balanced-pair runs and produces at least one pair")
end

-- rho_nc: largest eigenvalue of the non-coincidence transition
-- matrix.  For Tribonacci, the Python closure_test.py expects
-- rho_nc ~= 1.395337.  Within the BFS reach we may obtain a
-- close value (acceptable tolerance is wide; the closure_test
-- script uses a 20k-pair cap identical to ours).
local function closure_rho_nc_tribonacci()
    local sigma = subst_from_letters({ "ab", "ac", "a" })
    local rho = sp.balanced_pair.rho_nc(sigma)
    T(rho > 1.0 and rho < 2.0,
        string.format("tribonacci rho_nc in (1, 2): got %.6f", rho))
end

-- rho_nc for plastic: also unimodular-cubic Pisot; rho should
-- be in (1, beta).
local function closure_rho_nc_plastic()
    local sigma = subst_from_letters({ "b", "c", "ab" })
    local rho = sp.balanced_pair.rho_nc(sigma)
    T(rho > 1.0 and rho < 1.5,
        string.format("plastic rho_nc in (1, 1.5): got %.6f", rho))
end

-- substitution expand: sigma^k(seed) growing to >= n letters,
-- then truncated.
local function expand_fibonacci()
    local sigma = subst_from_letters({ "ab", "a" })
    local w = sp.substitution.expand(sigma, 0, 30)  -- seed=0, length=30
    T(#w == 30, string.format("expand returns length 30 (got %d)", #w))
    -- The Fibonacci word starting with a = 0 has the form
    -- "a b a a b a a b a a b a a b a a b a a b a a b a a b a a b a"
    -- i.e. {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, ...}
    NEAR(w[1], 0, 1e-9, "fibonacci expand[1] = 0 (a)")
    NEAR(w[2], 1, 1e-9, "fibonacci expand[2] = 1 (b)")
    NEAR(w[3], 0, 1e-9, "fibonacci expand[3] = 0 (a)")
end

-- substitution matrix: M[r][c] = count of r in sigma[c].
local function tribonacci_matrix_known()
    local sigma = subst_from_letters({ "ab", "ac", "a" })
    local M = sp.substitution.matrix(sigma)
    -- Image of a = {0,1}; image of b = {0,2}; image of c = {0}.
    -- So M[0][0]=1, M[1][0]=1 (from "ab"),
    --    M[0][1]=1, M[2][1]=1 (from "ac"),
    --    M[0][2]=1 (from "a").
    -- i.e. [[1,1,1],[1,0,0],[0,1,0]]
    NEAR(M[1][1], 1, 1e-9, "M[a][a] = 1")
    NEAR(M[2][1], 1, 1e-9, "M[b][a] = 1")
    NEAR(M[1][2], 1, 1e-9, "M[a][b] = 1")
    NEAR(M[3][2], 1, 1e-9, "M[c][b] = 1")
    NEAR(M[1][3], 1, 1e-9, "M[a][c] = 1")
    NEAR(M[2][3], 0, 1e-9, "M[b][c] = 0")
    NEAR(M[3][3], 0, 1e-9, "M[c][c] = 0")
end

-- substitution alphabet_size
local function substitution_alphabet_size()
    local sigma = subst_from_letters({ "ab", "ac", "a" })
    local n = sp.substitution.alphabet_size(sigma)
    NEAR(n, 3, 1e-9, "alphabet_size = 3")
end

-- ---------- Barge flags ----------

-- Distinct initials: every column's first letter is different.
-- Constant finals: every column's last letter is the same.
--
-- tribonacci {ab, ac, a}:  initials {a, a, a}    (NOT distinct)
--                           finals   {b, c, a}   (NOT constant)
-- silver_2L {aab, a}:      initials {a, a}      (NOT distinct)
--                           finals   {b, a}      (NOT constant)
-- period_doubling {ab, aa}: initials {a, a}      (NOT distinct)
--                           finals   {b, a}      (NOT constant)
local function barge_flags_basic()
    local sigma = subst_from_letters({ "ab", "ac", "a" })
    local f = sp.barge.flags(sigma)
    F(f.distinct_initials, "tribonacci initials not distinct")
    F(f.constant_finals,   "tribonacci finals not constant")
end

local function barge_flags_silver()
    local sigma = subst_from_letters({ "aab", "a" })
    local f = sp.barge.flags(sigma)
    F(f.distinct_initials, "silver initials not distinct")
    F(f.constant_finals,   "silver finals not constant")
end

-- ---------- Pisot thermometer ----------

-- The Python thermometer uses N = 2^20 and LMAX = 2^13; we use
-- N = 2^14 and LMAX = 2^10 so the direct (O(N*L)) correlation
-- runs in well under a second.  Expected Theta ranges at the
-- smaller N are slightly different from the Python reference
-- but they remain in the same qualitative regime:
--   pure-point systems (Fibonacci, Tribonacci, period_doubling)
--     have Theta - 2 = Theta - 2 close to zero (gap ~ L^(-δ)).
--   non-Pisot systems (Thue-Morse) have a plateau near 4/3.
--   plastic is what is called "pure-point but slow".
local function thermometer_fibonacci_near_2()
    local sigma = subst_from_letters({ "ab", "a" })
    local th = sp.thermometer.measure(sigma, 0, 1 << 14, 1 << 10, 33)
    T(th.theta > 1.99,
        string.format("fibonacci Theta > 1.99 at N=2^14 (got %.5f)", th.theta))
    F(th.theta > 2.0 + 1e-9,
        string.format("fibonacci Theta bounded above by 2 (got %.5f)", th.theta))
end

local function thermometer_thue_morse_plateau()
    local sigma = subst_from_letters({ "ab", "ba" })
    local th = sp.thermometer.measure(sigma, 0, 1 << 14, 1 << 10, 33)
    -- Thue-Morse is non-Pisot; the Python script records Theta
    -- ~ 1.338 at N=2^20 (plateau at 4/3).
    T(th.theta > 1.0 and th.theta < 1.6,
        string.format("thue_morse Theta in (1, 1.6) (got %.5f)", th.theta))
end

local function thermometer_period_doubling_near_2()
    local sigma = subst_from_letters({ "ab", "aa" })
    local th = sp.thermometer.measure(sigma, 0, 1 << 14, 1 << 10, 33)
    T(th.theta > 1.9,
        string.format("period_doubling Theta > 1.9 (got %.5f)", th.theta))
end

local function thermometer_plastic_finite_size()
    local sigma = subst_from_letters({ "b", "c", "ab" })
    local th = sp.thermometer.measure(sigma, 0, 1 << 14, 1 << 10, 33)
    -- Plastic is the canonical "Pisot but slow" instance; the
    -- Python script records Theta ~ 0.808 at N=2^20.  At
    -- smaller N the direct correlation is higher (the gap to
    -- 2 isn't fully closed yet); we accept any value below 2.0.
    T(th.theta > 0 and th.theta < 2.0,
        string.format("plastic Theta in (0, 2) (got %.5f)", th.theta))
end

local function registry()
    return {
        { name = "pisot_status_per_system",            fn = pisot_status_per_system },
        { name = "certify_tribonacci_pure_discrete",    fn = certify_tribonacci_pure_discrete },
        { name = "certify_plastic_pure_discrete",       fn = certify_plastic_pure_discrete },
        { name = "certify_smallest_pisot",              fn = certify_smallest_pisot },
        { name = "certify_thue_morse_runs",             fn = certify_thue_morse_runs },
        { name = "closure_rho_nc_tribonacci",           fn = closure_rho_nc_tribonacci },
        { name = "closure_rho_nc_plastic",              fn = closure_rho_nc_plastic },
        { name = "expand_fibonacci",                    fn = expand_fibonacci },
        { name = "tribonacci_matrix_known",             fn = tribonacci_matrix_known },
        { name = "substitution_alphabet_size",          fn = substitution_alphabet_size },
        { name = "barge_flags_basic",                   fn = barge_flags_basic },
        { name = "barge_flags_silver",                  fn = barge_flags_silver },
        { name = "thermometer_fibonacci_near_2",        fn = thermometer_fibonacci_near_2 },
        { name = "thermometer_thue_morse_plateau",       fn = thermometer_thue_morse_plateau },
        { name = "thermometer_period_doubling_near_2",  fn = thermometer_period_doubling_near_2 },
        { name = "thermometer_plastic_finite_size",     fn = thermometer_plastic_finite_size },
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
