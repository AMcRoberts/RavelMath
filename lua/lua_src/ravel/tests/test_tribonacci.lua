-- lua/tests/test_tribonacci.lua
--
-- Tribonacci Parikh-vector balanced-pair refinement. Mirrors
-- tests/test_tribonacci_balanced_pair.cpp.
--
-- This is the Parikh-vector starter documented in
-- docs/THEOREM_STATUS.md section 2; it is NOT the
-- Hollander-Solomyak / Barge-Diamond certification algorithm. It
-- passes the Tribonacci seed test but does NOT pass Fibonacci or
-- Thue-Morse; see the technical note notes.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local T = runner.assert_true
local F = runner.assert_false
local NEAR = runner.assert_near

local sub

local function letters(values)
    local out = {}
    for _, v in ipairs(values) do out[#out + 1] = string.char(v) end
    return table.concat(out)
end

local function same_matrix(a, b)
    local A, B = a:matrix(), b:matrix()
    for i = 1, #A do
        for j = 1, #A[i] do
            if A[i][j] ~= B[i][j] then return false end
        end
    end
    return true
end

local function same_letter_counts(pair, n)
    local c1 = sub.letter_counts(pair.left, n)
    local c2 = sub.letter_counts(pair.right, n)
    for i = 1, n do
        if c1[i] ~= c2[i] then return false end
    end
    return true
end

local function coincidences(pairs)
    local c = 0
    for _, p in ipairs(pairs) do
        if sub.is_coincidence(p) then c = c + 1 end
    end
    return c
end

local function fit_rate(values, first)
    local sx, sy, sxx, sxy = 0.0, 0.0, 0.0, 0.0
    local n = 0
    for i = first + 1, #values do
        if values[i] > 0.0 then
            local x = i - 1
            local y = math.log(values[i])
            sx = sx + x; sy = sy + y
            sxx = sxx + x * x; sxy = sxy + x * y
            n = n + 1
        end
    end
    if n < 2 then return 1.0 end
    local denom = n * sxx - sx * sx
    if denom == 0.0 then return 1.0 end
    return math.exp((n * sxy - sx * sy) / denom)
end

local function untwisted_boundary_dimension(alpha)
    local function eq(s) return 2.0 * alpha ^ (3.0 * s) + alpha ^ (4.0 * s) - 1.0 end
    local lo, hi = 0.0, 2.0
    for _ = 1, 100 do
        local mid = (lo + hi) / 2.0
        if eq(mid) > 0.0 then lo = mid else hi = mid end
    end
    return (lo + hi) / 2.0
end

local function run(name, rule)
    local seed = { left = letters({ 0, 1 }), right = letters({ 1, 0 }) }
    T(sub.is_balanced(seed, rule:alphabet_size()),
      string.format("%s seed is not balanced", name))
    local seed_parts = sub.split_balanced_pair(seed, rule:alphabet_size())
    T(#seed_parts == 1, string.format("%s anagram seed should be irreducible", name))

    local pairs = { seed }
    local noncoincidences = {}
    local max_level = 12
    print(string.format("%s", name))
    for level = 0, max_level do
        local cc = coincidences(pairs)
        for _, pair in ipairs(pairs) do
            T(same_letter_counts(pair, rule:alphabet_size()),
              string.format("%s refinement produced an unbalanced pair", name))
        end
        noncoincidences[level + 1] = #pairs - cc
        print(string.format("  level=%d pairs=%d coincidences=%d noncoincidences=%d",
            level, #pairs, cc, #pairs - cc))
        if level ~= max_level then
            pairs = sub.refine_balanced_pairs(rule, pairs)
        end
    end
    local beta = 1.839286755214161
    local alpha = math.sqrt(1.0 / beta)
    local rate = fit_rate(noncoincidences, 5)
    local reference_dimension = name == "untwisted"
        and untwisted_boundary_dimension(alpha) or 1.791903
    local required_rate = beta * alpha ^ (2.0 - reference_dimension)
    local measured_dimension = 2.0 - (math.log(beta) - math.log(rate)) / math.log(1.0 / alpha)
    print(string.format("  fitted noncoincidence rate=%.9f", rate))
    print(string.format("  reference boundary dimension=%.9f", reference_dimension))
    print(string.format("  rate required by excerpt identity=%.9f", required_rate))
    print(string.format("  dimension inferred from fitted rate=%.9f", measured_dimension))
end

local function matrix_test()
    local untwisted = sub.SubstitutionRule.new(
        sub.reencode({ "ab", "ac", "a" }))
    local twisted   = sub.SubstitutionRule.new(
        sub.reencode({ "ba", "ac", "a" }))
    T(same_matrix(untwisted, twisted), "twisted and untwisted matrices differ")

    local seed = { left = letters({ 0, 1 }), right = letters({ 1, 0 }) }
    local untwisted_image = {
        left = untwisted:apply_once(seed.left),
        right = untwisted:apply_once(seed.right),
    }
    local twisted_image = {
        left = twisted:apply_once(seed.left),
        right = twisted:apply_once(seed.right),
    }
    T(same_letter_counts(untwisted_image, 3), "untwisted image not balanced")
    T(same_letter_counts(twisted_image,   3), "twisted image not balanced")

    local up = sub.split_balanced_pair(untwisted_image, 3)
    local tp = sub.split_balanced_pair(twisted_image,   3)
    T(#up ~= #tp, "word order did not change the balanced decomposition")
    T(sub.is_coincidence(up[1]), "untwisted first factor is not a coincidence")
    F(sub.is_coincidence(tp[1]), "twisted first factor unexpectedly is a coincidence")

    run("untwisted", untwisted)
    run("twisted",   twisted)
    print("balanced-pair checks: PASS")
end

local function main(spectre_module)
    sub = spectre_module.substitution_rule
    TEST("tribonacci_balanced_pair_matrix_and_refinement", matrix_test)
    return R.run_all()
end

return { main = main }
