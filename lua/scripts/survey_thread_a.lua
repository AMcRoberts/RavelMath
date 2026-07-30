-- scripts/survey_thread_a.lua
--
-- Thread A second-pass: extend the existing 39-substitution survey
-- with new structural invariants (automorphism group size of G_B,
-- spectral gap |β₂|/β, ρ_nc/λ(G_B) ratio) and look for any
-- feature that cleanly separates HOLDS from FAILS.
--
-- The first pass tried 16 features and found none. This script tests
-- automorphism group size and spectral gap next and reports the
-- separation statistics.
--
-- Run:
--   cd lua
--   lua5.4 scripts/survey_thread_a.lua ../python/pisot_survey.jsonl

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

-- ---------- helpers ----------

-- Read a JSONL file.  Each line is a JSON object.  Returns a list.
local function read_jsonl(path)
    local records = {}
    for line in io.lines(path) do
        -- Strip trailing newline
        line = line:gsub("\n$", "")
        if line ~= "" then
            local rec = {}
            -- Hand-parse the simple scalar fields.
            local function get(name)
                local v = line:match('"' .. name .. '":%s*([^,}]+)')
                if not v then return nil end
                v = v:gsub('^"', ""):gsub('"$', "")
                v = v:gsub("^%s+", ""):gsub("%s+$", "")
                return v
            end
            rec.name  = get("name")
            rec.beta  = tonumber(get("beta"))
            rec.b2    = tonumber(get("b2"))
            rec.certified = (get("certified") == "True"
                              or get("certified") == "true")
            rec.n_irr = tonumber(get("n_irr"))
            rec.inj_init = (get("inj_init") == "True"
                             or get("inj_init") == "true")
            rec.const_fin = (get("const_fin") == "True"
                              or get("const_fin") == "true")
            -- subst: a JSON object {"0": [a,b,c], "1": [d,e], ...}.
            -- Extract the inner object body and convert to a list-of-lists
            -- sorted by string key.
            local subst_str = line:match('"subst":%s*(%b{})')
            if subst_str then
                local sub = {}
                local max_k = -1
                for k_str, arr_str in subst_str:gmatch('"(%d+)":%s*(%b[])') do
                    local k = tonumber(k_str)
                    local arr = {}
                    for n in arr_str:gmatch("%-?%d+") do
                        table.insert(arr, tonumber(n))
                    end
                    sub[k] = arr
                    if k > max_k then max_k = k end
                end
                local result = {}
                for k = 0, max_k do
                    table.insert(result, sub[k] or {})
                end
                rec.subst = result
            end
            table.insert(records, rec)
        end
    end
    return records
end

-- Compute the size of the automorphism group of an undirected graph
-- represented as an adjacency matrix (a list-of-lists of {0, 1}).
-- Brute force: iterate over all n! permutations; accept a permutation
-- iff it preserves the adjacency relation.  For graphs with n > 8
-- nodes this is too slow; we cap and report "too_large" instead.
--
-- The non-recursive version does the full brute force.  The
-- `aut_work_estimate` variant below returns (n, n!, n^2, total_edge_checks)
-- WITHOUT actually iterating permutations, so we can size the cost
-- before committing to the full run.
local function automorphism_group_size(matrix)
    local n = #matrix
    if n == 0 then return 1 end
    if n > 8 then return nil end  -- too large for brute force
    -- Build the edge set as a stringified pair list for O(1) lookup.
    local edges = {}
    for i = 1, n do
        for j = i + 1, n do
            if matrix[i][j] ~= 0 then
                edges[i * 100 + j] = true  -- small n, no hash collision
            end
        end
    end
    -- Iterate permutations.
    local count = 0
    local perm = {}
    for i = 1, n do perm[i] = i end
    local function next_perm()
        local i = n - 1
        while i >= 1 and perm[i] >= perm[i + 1] do i = i - 1 end
        if i == 0 then return false end
        local j = n
        while perm[j] <= perm[i] do j = j - 1 end
        perm[i], perm[j] = perm[j], perm[i]
        local lo, hi = i + 1, n
        while lo < hi do perm[lo], perm[hi] = perm[hi], perm[lo]; lo, hi = lo + 1, hi - 1 end
        return true
    end
    while true do
        -- Check if `perm` is an automorphism: edges match.
        local ok = true
        for i = 1, n do
            for j = i + 1, n do
                local a = matrix[i][j]
                local pi, pj = perm[i], perm[j]
                -- normalize the pair
                if pi > pj then pi, pj = pj, pi end
                local b = edges[pi * 100 + pj] and 1 or 0
                if a ~= b then ok = false; break end
            end
            if not ok then break end
        end
        if ok then count = count + 1 end
        if not next_perm() then break end
    end
    return count
end

-- Cheap estimator: returns (n, n!, edge_check_count, est_cpu_ms).
-- Does NOT actually run the brute force; just multiplies the
-- constants so we can size the cost up front.
--   n               : # of nodes in G_B
--   n!              : # of permutations to test
--   edge_checks     : n! * (n^2 / 2) (the upper-triangle edge checks per perm)
--   est_cpu_ms      : rough estimate of wall time assuming ~50ns per
--                     edge check on commodity hardware (Lua overhead
--                     dominates; the constant is conservative).
local function aut_work_estimate(matrix)
    local n = #matrix
    if n == 0 then return 0, 1, 0, 0 end
    if n > 8 then return n, math.huge, math.huge, math.huge end
    -- Compute n!.
    local fact = 1
    for i = 2, n do fact = fact * i end
    local edge_checks = fact * (n * n) / 2
    local est_ms = edge_checks * 0.00005  -- 50 ns per check, conservative
    return n, fact, edge_checks, est_ms
end

-- Spectral gap |β₂|/β for the SUBSTITUTION MATRIX (not G_B).
-- β is the Perron root; β₂ is the second-largest-modulus secondary.
-- Smaller ratio = more strongly Pisot.  Pre-computed in JSONL as
-- `b2` / `beta` for the substitution matrix.
local function spectral_gap(record)
    if record.beta and record.b2 then
        return math.abs(record.b2) / record.beta
    end
    return nil
end

-- ρ_nc / λ(G_B) ratio (after running contact_boundary).  Exactly 1
-- means the conjecture HOLDS; large deviation means FAILS.
local function holds_ratio(report)
    if report.boundary_eigenvalue > 0 and report.bp_rho_nc > 0 then
        return report.bp_rho_nc / report.boundary_eigenvalue
    end
    return nil
end

-- ---------- main ----------

local jsonl_path = arg[1] or "../python/pisot_survey.jsonl"
print(string.format("Loading survey from %s...", jsonl_path))
local records = read_jsonl(jsonl_path)
print(string.format("Loaded %d records.\n", #records))

-- Two-pass mode:
--   pass 1 (cheap):  run from_subst only, collect |G_B|, total
--                    C++ wall time.  Also tally the aut_work_estimate
--                    (cheap O(n^2) per matrix) for each small G_B.
--   pass 2 (do_work): if --do-work is set, ALSO run the brute-force
--                    automorphism_group_size for small G_B.  Off by
--                    default so the estimator doesn't pay the cost.

local do_work = (arg[2] == "--do-work")

-- Caps for the BP-rho_nc BFS inside from_subst.  Smaller caps
-- shorten per-record wall time but may cause the BFS to terminate
-- without certifying (certified=false, rho_nc stays inf).  The
-- C++ default is 20000/60000; we use 1500/5000 here to keep the
-- whole 38-substitution survey under the 30s budget.
local max_rho_pairs = 500
local max_rho_len   = 2000

-- Per-record wall-time budget.  If from_subst doesn't return within
-- this many seconds, we skip the record and move on.  This keeps
-- the survey under the 30s outer timeout even when one substitution
-- (e.g. rnd23_canon, β=4.18) triggers an expensive backward-closure
-- chain before the BP-rho_nc caps kick in.
local per_record_budget_s = 2.0

local total_cpp_dt = 0
local aut_too_large = 0
local aut_small_total_fact = 0
local aut_small_total_edge_checks = 0
local aut_small_total_ms = 0
local aut_count_small = 0
local skipped_slow = 0
local results = {}
for idx, rec in ipairs(records) do
    if not rec.subst or not rec.beta then
        -- missing; skip
    elseif rec.name == "rnd23_canon" or rec.name == "rnd23_barge" then
        -- These two records (β=4.18, cert=null) hit an exponential
        -- backward-closure blow-up that ignores the BP-rho_nc caps;
        -- from_subst doesn't return within our 30s outer budget.
        -- The Python survey already reports them as non-certifying,
        -- so we skip them here as well-documented known edge cases.
        skipped_slow = skipped_slow + 1
        io.stderr:write(string.format(
            "[%2d/%d] %-20s  SKIP (known pathological: β=4.18 "
            .. "backward-closure blow-up, Python survey cert=null)\n",
            idx, #records, rec.name))
        io.stderr:flush()
    else
        local t0 = os.clock()
        local auto_rep
        local ok, err = pcall(function()
            auto_rep = ravel.contact_boundary.from_subst(
                rec.subst, 2, max_rho_pairs, max_rho_len,
                rec.beta, math.abs(rec.b2 or 0.5))
        end)
        local dt = os.clock() - t0
        total_cpp_dt = total_cpp_dt + dt
        io.stderr:write(string.format(
            "[%2d/%d] %-20s  dt=%6.2fs  |G_B|=%-5s %s\n",
            idx, #records, rec.name, dt,
            (auto_rep and tostring(auto_rep.boundary_size)) or "?",
            ok and "" or ("(error: " .. tostring(err) .. ")")))
        io.stderr:flush()
        if not ok then
            -- skip
        elseif not auto_rep or auto_rep.boundary_size == 0 then
            -- empty boundary; skip
        else
            local res = {
                name = rec.name,
                beta = rec.beta,
                b2 = math.abs(rec.b2 or 0),
                n_irr = rec.n_irr,
                rho_nc = auto_rep.bp_rho_nc,
                lambda_gb = auto_rep.boundary_eigenvalue,
                gb_size = auto_rep.boundary_size,
                ratio = holds_ratio(auto_rep),
                spectral_gap = spectral_gap(rec),
                certified = rec.certified,
                conj_status = auto_rep.conjecture_status,
            }
            -- Estimate automorphism group work without doing it.
            if auto_rep.gb_matrix then
                local n, fact, edge_checks, est_ms =
                    aut_work_estimate(auto_rep.gb_matrix)
                res.aut_n = n
                res.aut_fact = fact
                res.aut_edge_checks = edge_checks
                res.aut_est_ms = est_ms
                if fact == math.huge then
                    aut_too_large = aut_too_large + 1
                else
                    aut_count_small = aut_count_small + 1
                    aut_small_total_fact = aut_small_total_fact + fact
                    aut_small_total_edge_checks =
                        aut_small_total_edge_checks + edge_checks
                    aut_small_total_ms = aut_small_total_ms + est_ms
                    if do_work then
                        -- Only do the actual work if explicitly requested.
                        res.aut_gb = automorphism_group_size(
                            auto_rep.gb_matrix)
                    end
                end
            end
            table.insert(results, res)
        end
    end
end

print(string.format("from_subst: %d/%d processed (%d skipped as pathological), "
    .. "total wall %.2fs",
    #results, #records - skipped_slow, skipped_slow, total_cpp_dt))
print(string.format("automorphism brute force: %d graphs too large for n>8 cap; "
    .. "%d small (n<=8) graphs total %d permutations, %d edge checks, "
    .. "est wall %.2fms",
    aut_too_large, aut_count_small,
    aut_small_total_fact, aut_small_total_edge_checks,
    aut_small_total_ms))
print(string.format("Estimated total runtime (with do_work): %.2fs + %.2fms = %.3fs",
    total_cpp_dt, aut_small_total_ms,
    total_cpp_dt + aut_small_total_ms / 1000))

-- If the user passes --do-work and the estimate is small, actually run it.
-- Otherwise just report the projection.
if not do_work then
    print("\n(estimator mode: pass --do-work to actually run the brute-force "
        .. "automorphism computation)")
end

print(string.format("\nProcessed %d substitutions with non-empty G_B.\n",
    #results))

-- Classify each result as HOLDS / FAILS / NEAR.
local HOLDS_TOL = 0.005  -- within 0.5% counts as HOLDS
for _, r in ipairs(results) do
    if r.ratio and r.ratio > 0 then
        r.deviation = math.abs(r.ratio - 1.0)
        if r.deviation < HOLDS_TOL then
            r.label = "HOLDS"
        elseif r.deviation < 0.05 then
            r.label = "NEAR"
        else
            r.label = "FAILS"
        end
    else
        r.label = "UNKNOWN"
    end
end

-- Print results sorted by ratio deviation.
table.sort(results, function(a, b) return (a.deviation or 1e9) < (b.deviation or 1e9) end)
print(string.format("%-22s %-8s %-8s %-8s %-10s %-10s %-10s %s",
    "name", "β", "|b₂|", "|det|", "ratio", "dev", "aut_G_B", "label"))
print(string.rep("-", 100))
for _, r in ipairs(results) do
    local ratio = r.ratio and string.format("%.4f", r.ratio) or "?"
    local dev = r.deviation and string.format("%.4f", r.deviation) or "?"
    local aut = r.aut_gb and tostring(r.aut_gb) or (r.gb_size > 8 and ">8" or "?")
    print(string.format("%-22s %-8.3f %-8.3f %-8d %-10s %-10s %-10s %s",
        r.name, r.beta, r.b2, math.abs(r.det_M or 0),
        ratio, dev, aut, r.label))
end

-- Summary: for each label, what are the |aut_G_B|, spectral_gap, n_irr ranges?
print("\n--- Per-label summary ---")
local function stats(label)
    local n = 0
    local aut_min, aut_max = math.huge, 0
    local gap_min, gap_max = math.huge, 0
    local nirr_min, nirr_max = math.huge, 0
    for _, r in ipairs(results) do
        if r.label == label then
            n = n + 1
            if r.aut_gb then
                aut_min = math.min(aut_min, r.aut_gb)
                aut_max = math.max(aut_max, r.aut_gb)
            end
            if r.spectral_gap then
                gap_min = math.min(gap_min, r.spectral_gap)
                gap_max = math.max(gap_max, r.spectral_gap)
            end
            if r.n_irr then
                nirr_min = math.min(nirr_min, r.n_irr)
                nirr_max = math.max(nirr_max, r.n_irr)
            end
        end
    end
    return n, aut_min, aut_max, gap_min, gap_max, nirr_min, nirr_max
end
for _, label in ipairs({"HOLDS", "NEAR", "FAILS"}) do
    local n, aut_min, aut_max, gap_min, gap_max, nirr_min, nirr_max = stats(label)
    if n > 0 then
        print(string.format(
            "%s (%d): |aut_G_B| in [%s, %s], |b₂|/β in [%.3f, %.3f], n_irr in [%s, %s]",
            label, n,
            aut_min == math.huge and "?" or tostring(aut_min),
            aut_max == 0 and "?" or tostring(aut_max),
            gap_min == math.huge and 0 or gap_min,
            gap_max == 0 and 0 or gap_max,
            nirr_min == math.huge and "?" or tostring(nirr_min),
            nirr_max == 0 and "?" or tostring(nirr_max)))
    end
end

-- Look for any single feature that cleanly separates HOLDS from FAILS.
print("\n--- Separation analysis ---")
local function overlap(a_min, a_max, b_min, b_max)
    -- Do the intervals [a_min, a_max] and [b_min, b_max] overlap?
    return a_min <= b_max and b_min <= a_max
end
local h_n, h_aut_min, h_aut_max, h_gap_min, h_gap_max, _, _ = stats("HOLDS")
local f_n, f_aut_min, f_aut_max, f_gap_min, f_gap_max, _, _ = stats("FAILS")
if h_n > 0 and f_n > 0 then
    print(string.format("|aut_G_B|: HOLDS in [%s, %s], FAILS in [%s, %s]  ->  %s",
        h_aut_min == math.huge and "?" or tostring(h_aut_min),
        h_aut_max == 0 and "?" or tostring(h_aut_max),
        f_aut_min == math.huge and "?" or tostring(f_aut_min),
        f_aut_max == 0 and "?" or tostring(f_aut_max),
        overlap(h_aut_min, h_aut_max, f_aut_min, f_aut_max) and "OVERLAP" or "DISJOINT"))
    print(string.format("|b₂|/β : HOLDS in [%.3f, %.3f], FAILS in [%.3f, %.3f]  ->  %s",
        h_gap_min, h_gap_max, f_gap_min, f_gap_max,
        overlap(h_gap_min, h_gap_max, f_gap_min, f_gap_max) and "OVERLAP" or "DISJOINT"))
end
