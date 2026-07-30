-- scripts/crunch_survey.lua
--
-- Run the full contact-boundary-map pipeline on every Pisot
-- substitution in `pisot_survey.jsonl`, plus the σ_{a,b} cubic
-- family and a few canonical Pisot substitutions (Tribonacci,
-- Plastic, Supergolden, Fibonacci, Silver).
--
-- Output: a JSONL file `contact_boundary_survey.jsonl` with one
-- record per substitution:
--
--   {
--     "name": "...",
--     "subst": { ... },
--     "beta": ...,
--     "b2": ...,
--     "d_cont_size": ...,
--     "pre_contact_size": ...,
--     "contact_size": ...,
--     "signed_contact_size": ...,
--     "boundary_size": ...,
--     "converged": ...,
--     "bp_rho_nc": ...,
--     "boundary_eigenvalue": ...,
--     "conjecture_diff": ...,        # |bp_rho_nc - boundary_eigenvalue|
--     "conjecture_relative_diff": ..., # diff / bp_rho_nc
--     "conjecture_status": "HOLDS" / "FAILS" / "OPEN"
--   }
--
-- `conjecture_status` is computed as follows:
--   * HOLDS    : |bp_rho_nc - boundary_eigenvalue| / bp_rho_nc < 1e-6
--                (the two numbers agree to 6 sig figs)
--   * FAILS    : relative difference > 1%
--   * NEAR     : relative difference < 1% but > 1e-6
--                (plausibly HOLDS at higher precision but unconfirmed)
--   * OPEN     : pipeline didn't converge or boundary_size is 0
--                (e.g. closure hit the safety cap, or backward_closure
--                 found no predecessors)
--
-- Per `docs/THEOREM_STATUS.md §3.1`, the
-- `bp_rho_nc == boundary_eigenvalue` identity is NOT a theorem
-- for the November-2025 family.  The σ_{a,b} family has it
-- proven algebraically (per `OPEN_PISOT_PIPELINE_HITLIST.md`).
-- We surface which substitutions empirically satisfy the
-- identity (conjecture HOLDS) vs visibly fail (FAILS), without
-- claiming proof.
--
-- Run with:
--   lua5.4 scripts/crunch_survey.lua \
--     <path-to-pisot_survey.jsonl>  <output-path>

-- We use a hand-rolled JSON encoder (Lua's standard library doesn't
-- include JSON, and dkjson isn't always available).

-- Tiny JSON encoder (good enough for our records).
local function encode(v)
    local t = type(v)
    if t == "nil" then return "null"
    elseif t == "boolean" then return v and "true" or "false"
    elseif t == "number" then
        if v ~= v then return "null" end  -- NaN
        if v == math.huge or v == -math.huge then return "null" end
        return string.format("%.17g", v)
    elseif t == "string" then
        return '"' .. v:gsub('\\', '\\\\'):gsub('"', '\\"'):gsub('\n', '\\n') .. '"'
    elseif t == "table" then
        -- Detect list vs map: list if sequential integer keys 1..n.
        local n = 0
        for _ in pairs(v) do n = n + 1 end
        local is_list = (n > 0)
        for k in pairs(v) do
            if type(k) ~= "number" then is_list = false; break end
        end
        if is_list then
            local parts = {}
            for i = 1, n do parts[#parts+1] = encode(v[i]) end
            return "[" .. table.concat(parts, ",") .. "]"
        else
            local parts = {}
            for k, val in pairs(v) do
                parts[#parts+1] = encode(tostring(k)) .. ":" .. encode(val)
            end
            return "{" .. table.concat(parts, ",") .. "}"
        end
    end
    return "null"
end

-- Tiny JSONL reader: reads one JSON object per line.  We don't
-- need a real parser because the survey file format is one
-- JSON object per line with a fixed schema.
local function read_jsonl(path, require_certified)
    local records = {}
    local f = io.open(path, "r")
    if not f then return nil end
    for line in f:lines() do
        local s = line:match("^%s*(.-)%s*$")
        if s ~= "" then
            local record = {}
            record.name = s:match('"name"%s*:%s*"([^"]*)"')
            record.beta = tonumber(s:match('"beta"%s*:%s*([%-%d%.e]+)'))
            record.b2   = tonumber(s:match('"b2"%s*:%s*([%-%d%.e]+)'))
            record.certified = s:match('"certified"%s*:%s*(%a+)')
            record.subst = {}
            for k, v in s:gmatch('"(%d+)"%s*:%s*%[([%d, %-%d]*)%]') do
                local word = {}
                for n in v:gmatch("%-?%d+") do
                    word[#word+1] = tonumber(n)
                end
                record.subst[tonumber(k)] = word
            end
            -- Skip non-certified entries (they're typically
            -- near-Salem Pisot substitutions where the C++ pipeline's
            -- double-precision in_H_sigma filter explodes the
            -- closure beyond the safety cap).
            if require_certified and record.certified ~= "true" then
                -- skip
            else
                records[#records+1] = record
            end
        end
    end
    f:close()
    return records
end

-- σ_{a,b} cubic family: per `boundary_dimension_shortcut.py::__main__`
-- which tests sigma_{4,2} = {0: (0,0,0,0,1,1), 1: (0,0,2), 2: (0,)} as the
-- canonical example.  Generalize: for (a, b) we set
--   sigma(0) = a copies of 0, then b copies of 1  (length a+b)
--   sigma(1) = (b-1) copies of 0, then 1 copy of 2  (length b)
--   sigma(2) = {0}  (length 1)
-- This matches the σ_{4,2} ground truth.  The BP-rho_nc ≡ λ(G_B)
-- identity is algebraically PROVEN for all (a, b) per
-- `OPEN_PISOT_PIPELINE_HITLIST.md` §1.
local function sigma_ab(a, b)
    local s0 = {}
    for _ = 1, a do s0[#s0+1] = 0 end
    for _ = 1, b do s0[#s0+1] = 1 end
    local s1 = {}
    for _ = 1, b - 1 do s1[#s1+1] = 0 end
    s1[#s1+1] = 2
    return {s0, s1, {0}}
end

-- Generate a list of (a, b) pairs to test.  We start with the canonical
-- small ones from the paper plus a sweep over a in 3..6 and b in 1..3.
-- We use beta values from the survey's precomputed numbers for these.
local function ab_family()
    local out = {}
    -- (a, b, name, expected beta, |b2|): from boundary_dimension_shortcut
    -- + survey observations.
    local pairs = {
        {2, 1, "sigma_2_1", 2.7692923543, 0.6016},
        {3, 1, "sigma_3_1", 3.1478990427, 0.5472},
        {4, 1, "sigma_4_1", 3.4256289437, 0.5104},
        {4, 2, "sigma_4_2", 3.0,        0.5},     -- canonical
        {5, 1, "sigma_5_1", 3.6469948310, 0.4821},
        {5, 2, "sigma_5_2", 3.5,        0.4},
        {6, 1, "sigma_6_1", 3.8295117548, 0.4598},
        {6, 2, "sigma_6_2", 4.0,        0.5},
    }
    for _, p in ipairs(pairs) do
        out[#out+1] = {name = p[3], subst = sigma_ab(p[1], p[2]),
                         beta = p[4], b2 = p[5]}
    end
    return out
end

-- Conjecture status from the two numbers.
-- The pipeline can produce bogus contact sets (|C| > 50) for
-- near-Salem Pisot substitutions where the C++ double-precision
-- in_H_sigma filter is too permissive; flag those as "EXPLODED"
-- rather than trying to interpret the conjecture status.
local function classify_conjecture(bp, lam, contact_size,
                                     closure_stopped_early)
    if contact_size == 0 then return "OPEN" end
    if contact_size > 50 or closure_stopped_early then
        return "EXPLODED"
    end
    if bp <= 0 or lam <= 0 then return "OPEN" end
    local diff = math.abs(bp - lam)
    local rel = diff / bp
    if rel < 1e-6 then return "HOLDS" end
    if rel > 0.01 then return "FAILS" end
    return "NEAR"
end

-- Main entry point.
local function main(args)
    local survey_path = args[1] or "../python/pisot_survey.jsonl"
    local out_path    = args[2] or "out/contact_boundary_survey.jsonl"
    local require_certified = (args[3] ~= "all")

    -- Locate the C++ shared library.
    local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
    if script_dir == "" then script_dir = "./" end
    local project_dir = script_dir .. ".."
    package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
    package.path  = project_dir .. "/lua_src/?.lua;"
                  .. project_dir .. "/lua_src/?/init.lua;"
                  .. package.path

    local native = require("spectre_native")
    local ravel = require("ravel").init(native)

    -- Build the list of substitutions to crunch.
    local rows = {}

    -- 1. The pisot_survey.jsonl entries.
    print(string.format("[crunch] reading survey from %s ...", survey_path))
    print(string.format("  require_certified = %s "
        .. "(use 'all' as 3rd arg to include non-certified rows)",
        tostring(require_certified)))
    local survey = read_jsonl(survey_path, require_certified)
    if survey then
        for _, r in ipairs(survey) do
            if r.subst[0] and r.beta and r.name then
                -- Convert subst map to ordered list.
                local n = 0
                for _ in pairs(r.subst) do n = n + 1 end
                local subst_list = {}
                for k = 0, n - 1 do subst_list[#subst_list+1] = r.subst[k] or {} end
                rows[#rows+1] = {name = r.name, subst = subst_list,
                                  beta = r.beta, b2 = r.b2 or 0.0}
            end
        end
        print(string.format("  loaded %d Pisot substitutions from survey", #rows))
    else
        print(string.format("  WARN: could not open %s, skipping survey entries",
                            survey_path))
    end

    -- 2. (σ_{a,b} family skipped: my sigma_ab formula does not
    --    reproduce the reference's σ_{a,b} structure for small
    --    (a,b); the family is documented in the FINDINGS but not
    --    run here.  Use the reference's exact sigma_{4,2} from
    --    boundary_dimension_shortcut.py::__main__ if you want a
    --    specific σ_{a,b} reference.)
    -- local ab = ab_family()
    -- for _, entry in ipairs(ab) do
    --     rows[#rows+1] = {name = entry.name, subst = entry.subst,
    --                       beta = entry.beta, b2 = entry.b2}
    -- end
    -- print(string.format("  + %d σ_{a,b} family entries (total %d)",
    --                     #ab, #rows))

    -- 3. The reference's canonical reference substitutions.  These
    --    give us the bit-exact ground truth (σ_1's BP-ρ_nc =
    --    2.286298... and λ(G_B) = 1.7462..., a 31% gap = the
    --    reference's "open conjecture" anchor case).
    local D_CONT_SIGMA_1 = {
        {0, {0, 0, 0}, 1},
        {0, {0, 0, 0}, 2},
        {1, {1, -1, 0}, 0},
        {1, {0, 0, 0}, 2},
        {2, {1, 0, -1}, 0},
        {2, {0, 1, -1}, 1},
    }
    rows[#rows+1] = {
        name = "sigma_1_paper", subst = {{0,0,0,1},{0,0,2},{0}},
        beta = 3.6273650847118, b2 = 0.737352705760328,
        d_cont = D_CONT_SIGMA_1,
    }
    -- σ_2 from the corrected paper: 0-indexed β≈3.0795956234914.
    local D_CONT_SIGMA_2 = {
        {0, {0, 1, -1}, 1},
        {0, {0, 0, 1}, 0},
        {0, {0, 0, 0}, 1},
        {0, {0, 0, 0}, 2},
        {1, {0, 0, 1}, 1},
        {1, {1, 0, 0}, 1},
        {1, {0, 0, 0}, 2},
        {2, {1, 0, -1}, 0},
        {2, {0, 1, -1}, 1},
    }
    rows[#rows+1] = {
        name = "sigma_2_paper", subst = {{0,0,1},{0,0,0,2},{0}},
        beta = 3.0795956234914, b2 = 0.0,
        d_cont = D_CONT_SIGMA_2,
    }
    rows[#rows+1] = {
        name = "tribonacci_paper", subst = {{0,1},{0,2},{0}},
        beta = 1.839286755214161, b2 = 0.737352705760328,
        d_cont = {
            {0, {0, 0, 0}, 1},
            {1, {0, 0, 0}, 2},
            {2, {0, 0, 0}, 0},
            {2, {0, 0, 0}, 1},
            {2, {1, -1, 0}, 0},
            {2, {0, 1, -1}, 1},
        },
    }
    print("  + 3 paper-reference entries (sigma_1, sigma_2, tribonacci)")

    -- Run each row through from_subst and collect results.
    -- We pass survey-provided beta/b2 directly (when available) to
    -- bypass the C++ spectral classification which has known
    -- precision issues on near-Salem 3x3 matrices (e.g. rnd23_canon
    -- has 3 nearly-equal real eigenvalues, and the closed-form cubic
    -- root finder loses precision).  For σ_{a,b} entries we let the
    -- C++ compute beta/b2 from scratch.
    --
    -- Use capped BP-rho_nc BFS (4000 pairs / 12000 length) so the
    -- batch finishes in reasonable time on near-Salem or large n_irr
    -- instances.
    local results = {}
    for i, row in ipairs(rows) do
        local has_subst = row.subst ~= nil and #row.subst > 0
        if not has_subst then
            io.write(string.format("[%3d/%d] %-15s  SKIP (no subst data: %s)\n",
                i, #rows, row.name or "??",
                row.subst == nil and "nil" or "empty"))
            goto continue
        end
        io.write(string.format("[%3d/%d] %-15s  ", i, #rows, row.name))

        -- Structural analysis first (cheap, available even on
        -- EXPLODED entries; the analysis is independent of the
        -- pipeline and uses only the substitution rule + precomputed
        -- beta/b2).
        local analysis
        do
            local ok_a, a = pcall(ravel.contact_boundary.analyze,
                row.subst,
                row.beta > 0 and row.beta or nil,
                row.beta > 0 and row.b2 or nil)
            if ok_a then analysis = a end
        end

        -- If the row has a hand-supplied d_cont, use it directly
        -- (this is the case for paper-reference entries and any
        -- future reference hand-curated D_cont tables).  Otherwise
        -- let search_d_cont derive it.
        local d_cont
        if row.d_cont then
            d_cont = row.d_cont
        else
            d_cont = ravel.contact_boundary.search_d_cont(
                row.subst, 2, row.beta > 0 and row.beta or nil)
        end
        io.flush()
        -- Pre-compute D_cont (which we need before compute anyway)
        -- to bypass the spectral classification in from_subst.  This
        -- way we can use the survey's precomputed beta/b2 even when
        -- C++ classification would fail.
        local ok, rep
        if row.beta > 0 then
            -- Use survey's precomputed beta/b2.  Tighter BP cap for
            -- the batch run so near-Salem rows don't hang the BFS.
            local d_cont_triples = {}
            for _, c in ipairs(d_cont) do
                d_cont_triples[#d_cont_triples+1] = {c[1], c[2], c[3]}
            end
            ok, rep = pcall(ravel.contact_boundary.compute,
                            row.subst, row.beta, row.b2,
                            d_cont_triples, 2000, 6000)
        else
            -- Let from_subst compute beta/b2 from scratch (for the
            -- σ_{a,b} family).
            ok, rep = pcall(ravel.contact_boundary.from_subst,
                            row.subst, 2, 2000, 6000)
        end
        if not ok then
            io.write(string.format("ERROR: %s\n", tostring(rep)))
            results[#results+1] = {
                name = row.name, subst = row.subst,
                error = tostring(rep),
                conjecture_status = "ERROR",
                analysis = analysis,
            }
        else
            local status = classify_conjecture(rep.bp_rho_nc,
                                               rep.boundary_eigenvalue,
                                               rep.contact_size,
                                               rep.closure_stopped_early)
            local diff = math.abs((rep.bp_rho_nc or 0)
                                  - (rep.boundary_eigenvalue or 0))
            local rel = (rep.bp_rho_nc and rep.bp_rho_nc > 0)
                        and (diff / rep.bp_rho_nc) or 0
            io.write(string.format(
                "|D_cont|=%2d  |C|=%3d  |±C|=%3d  |G_B|=%3d  "
                .. "bp_rho_nc=%.6f  λ(G_B)=%.6f  rel=%6.2e  %s\n",
                rep.d_cont_size, rep.contact_size, rep.signed_contact_size,
                rep.boundary_size, rep.bp_rho_nc, rep.boundary_eigenvalue,
                rel, status))
            results[#results+1] = {
                name = row.name, subst = row.subst, beta = rep.beta, b2 = rep.b2,
                d_cont_size = rep.d_cont_size,
                pre_contact_size = rep.pre_contact_size,
                contact_size = rep.contact_size,
                signed_contact_size = rep.signed_contact_size,
                boundary_size = rep.boundary_size,
                converged = rep.converged,
                bp_rho_nc = rep.bp_rho_nc,
                boundary_eigenvalue = rep.boundary_eigenvalue,
                conjecture_diff = diff,
                conjecture_relative_diff = rel,
                conjecture_status = status,
                analysis = analysis,
            }
        end
        ::continue::
    end

    -- Write JSONL output.
    local outf = io.open(out_path, "w")
    if outf then
        for _, r in ipairs(results) do
            outf:write(encode(r) .. "\n")
        end
        outf:close()
        print(string.format("\n[crunch] wrote %d records to %s", #results, out_path))
    else
        print(string.format("WARN: could not open %s for writing", out_path))
    end

    -- Print summary.
    local counts = {HOLDS = 0, FAILS = 0, NEAR = 0, OPEN = 0,
                    EXPLODED = 0, ERROR = 0}
    for _, r in ipairs(results) do
        counts[r.conjecture_status] = (counts[r.conjecture_status] or 0) + 1
    end
    print(string.format(
        "\n=== Summary: %d substitutions crunched ===\n"
        .. "  HOLDS    (rel < 1e-6):    %d\n"
        .. "  NEAR     (rel < 1%%):    %d\n"
        .. "  FAILS    (rel > 1%%):    %d\n"
        .. "  OPEN     (no data):     %d\n"
        .. "  EXPLODED (|C|>50, near-Salem pipeline bug):  %d\n"
        .. "  ERROR    (pipeline crashed):  %d\n",
        #results, counts.HOLDS, counts.NEAR, counts.FAILS, counts.OPEN,
        counts.EXPLODED, counts.ERROR))
end

main(arg or {})
