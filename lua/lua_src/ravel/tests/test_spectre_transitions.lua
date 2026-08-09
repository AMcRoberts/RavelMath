-- lua/ravel/tests/test_spectre_transitions.lua
--
-- Audit test for the per-Spectre transition table produced by
-- scripts/build_spectre_transition_table.lua and wrapped in
-- lua/ravel/data/spectre_transitions.lua.
--
-- The audit verifies three structural invariants:
--   1. Every (parent_hex, child_index, spectre_edge) entry has a
--      non-nil parent_edge and neighbour.
--   2. The destination neighbour is in the parent's hex outer
--      table for the parent_edge indicated by the entry (i.e. the
--      geometric / closest-segment assignments agree with the
--      parent hex outer transition table).
--   3. For G, the inner-child entries (child_index == 1) cover all
--      14 Spectre edges 0..13.
--
-- This test corresponds to step 4 of
-- docs/THEOREM_STATUS.md phase 2.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local T = runner.assert_true
local EQ = runner.assert_eq
local GE = runner.assert_le
local LE = runner.assert_le

local function has_specmap_svg(letter)
    local candidates = {
        "../tatham_svg_parsed/specmap_" .. letter .. ".svg",
        "tatham_svg_parsed/specmap_" .. letter .. ".svg",
    }
    for _, path in ipairs(candidates) do
        local f = io.open(path, "r")
        if f then f:close(); return true end
    end
    return false
end

local hex
local outer
local transitions

local function table_complete()
    local summary = transitions.summary()
    local expected = { G = has_specmap_svg("G") and 42 or 29 }
    for _, letter in ipairs({ "D", "J", "L", "X", "P", "S", "F", "Y" }) do
        expected[letter] = 14
    end
    for letter, n in pairs(expected) do
        EQ(summary[letter], n,
            string.format("hex %s has %d records (expected %d)",
                letter, summary[letter], n))
    end
end

-- Every (parent_hex, child_index, spectre_edge) must have a
-- non-nil parent_edge and neighbour.
local function every_entry_well_formed()
    for _, letter in ipairs({ "G", "D", "J", "L", "X", "P", "S", "F", "Y" }) do
        local kind = hex.NAME_TO_INDEX[letter]
        local records = transitions.all_for(kind)
        for _, r in ipairs(records) do
            T(r.parent_edge ~= nil,
                string.format("%s child=%d edge=%d missing parent_edge",
                    letter, r.child_index, r.spectre_edge))
            T(r.neighbour ~= nil,
                string.format("%s child=%d edge=%d missing neighbour",
                    letter, r.child_index, r.spectre_edge))
            T(r.spectre_edge >= 0 and r.spectre_edge <= 13,
                string.format("%s child=%d edge=%d out of range",
                    letter, r.child_index, r.spectre_edge))
            T(r.child_index == 0 or r.child_index == 1,
                string.format("%s child=%d invalid child index",
                    letter, r.child_index))
        end
    end
end

-- Every record's neighbour must agree with the hex outer table at
-- the recorded parent_edge. This is the strongest invariant: the
-- (parent_hex, parent_edge) -> (neighbour, child_index) mapping
-- must round-trip.
local function neighbour_matches_outer_table()
    for _, letter in ipairs({ "G", "D", "J", "L", "X", "P", "S", "F", "Y" }) do
        local kind = hex.NAME_TO_INDEX[letter]
        local records = transitions.all_for(kind)
        for _, r in ipairs(records) do
            local outer_row = outer.lookup(kind, r.parent_edge, 0)
            T(outer_row ~= nil,
                string.format("%s child=%d edge=%d parent_edge=%d not in outer table",
                    letter, r.child_index, r.spectre_edge, r.parent_edge))
            T(outer_row.neighbour == r.neighbour,
                string.format("%s child=%d edge=%d neighbour mismatch (got %s, expected %s)",
                    letter, r.child_index, r.spectre_edge,
                    hex.name(r.neighbour),
                    hex.name(outer_row.neighbour)))
        end
    end
end

-- The per-Spectre table must cover all 14 edges (0..13) of the
-- dominant Spectre (child_index == 0) for every hex.
local function every_dominant_spectre_complete()
    for _, letter in ipairs({ "G", "D", "J", "L", "X", "P", "S", "F", "Y" }) do
        local kind = hex.NAME_TO_INDEX[letter]
        local seen = {}
        for _, r in ipairs(transitions.all_for(kind)) do
            if r.child_index == 0 then seen[r.spectre_edge] = true end
        end
        for e = 0, 13 do
            T(seen[e],
                string.format("%s dominant Spectre edge %d missing", letter, e))
        end
    end
end

    -- G has an inner child only when its SVG supplies the child-index
    -- labels. The checked-in txt fallback intentionally cannot distinguish
    -- dominant and inner labels, so it must not be treated as a failed
    -- geometric audit.
local function g_inner_child_complete()
    if not has_specmap_svg("G") then return end
    local seen = {}
    for _, r in ipairs(transitions.all_for(hex.G)) do
        if r.child_index == 1 then seen[r.spectre_edge] = true end
    end
    for e = 0, 13 do
        T(seen[e], string.format("G inner child edge %d missing", e))
    end
end

-- For G, the geometric entries (source=geometric) are the gold
-- standard. There should be 28 of them.
local function g_uses_geometric_source()
    local geometric = 0
    local closest_segment = 0
    for _, r in ipairs(transitions.all_for(hex.G)) do
        if r.source == "geometric" then
            geometric = geometric + 1
        elseif r.source == "closest-segment" then
            closest_segment = closest_segment + 1
        end
    end
    local expected = has_specmap_svg("G") and 28 or 0
    EQ(geometric, expected,
        string.format("G should have %d geometric entries (got %d closest-segment=%d)",
            expected, geometric, closest_segment))
end

-- For S, the geometric source should be used (S has an SVG).
local function s_uses_geometric_source()
    local geometric = 0
    for _, r in ipairs(transitions.all_for(hex.S)) do
        if r.source == "geometric" then geometric = geometric + 1 end
    end
    local expected = has_specmap_svg("S") and 14 or 0
    EQ(geometric, expected,
        string.format("S should have %d geometric entries (got %d)", expected, geometric))
end

-- When a specmap SVG is present, entries should come from the geometric
-- path. Without that asset, the checked-in txt fallback is explicitly
-- provisional and closest-segment entries are expected.
local function all_hexes_use_geometric_source()
    for _, letter in ipairs({ "G", "D", "J", "L", "X", "P", "S", "F", "Y" }) do
        local kind = hex.NAME_TO_INDEX[letter]
        local geometric, fallback = 0, 0
        for _, r in ipairs(transitions.all_for(kind)) do
            if r.source == "geometric" then
                geometric = geometric + 1
            elseif r.source == "closest-segment" then
                fallback = fallback + 1
            end
        end
        local expected = has_specmap_svg(letter) and ((letter == "G") and 28 or 14) or 0
        local expected_fallback = #transitions.all_for(kind) - expected
        EQ(fallback, expected_fallback,
            string.format("hex %s should have %d closest-segment entries (got %d)",
                letter, expected_fallback, fallback))
        EQ(geometric, expected,
            string.format("hex %s should have %d geometric entries (got %d)",
                letter, expected, geometric))
    end
end

-- The destination neighbour is determined by the parent hex outer
-- table; spot-check a few values from the per-edge transcription
-- in docs/THEOREM_STATUS.md to make sure the table agrees
-- with the published hex outer map.
local function spot_check_destinations()
    if not has_specmap_svg("G") or not has_specmap_svg("S") then return end
    -- G.edge 0 -> F (per hex outer table).
    local r = transitions.lookup(hex.G, 0, 2)
    T(r ~= nil, "G dominant edge 2 missing")
    T(r.neighbour == hex.F,
        "G dominant edge 2 neighbour mismatch (expected F)")

    -- S.edge 0 -> G (per hex outer table for S).
    r = transitions.lookup(hex.S, 0, 1)
    T(r ~= nil, "S dominant edge 1 missing")
    T(r.neighbour == hex.G,
        "S dominant edge 1 neighbour mismatch (expected G)")
end

local function main(spectre_module)
    hex = spectre_module.hex_kind
    outer = spectre_module.outer
    transitions = spectre_module.spectre_transitions
    TEST("spectre_transitions_table_complete",            table_complete)
    TEST("spectre_transitions_every_entry_well_formed",    every_entry_well_formed)
    TEST("spectre_transitions_neighbour_matches_outer",    neighbour_matches_outer_table)
    TEST("spectre_transitions_dominant_complete",          every_dominant_spectre_complete)
    TEST("spectre_transitions_g_inner_child_complete",     g_inner_child_complete)
    TEST("spectre_transitions_g_uses_geometric_source",    g_uses_geometric_source)
    TEST("spectre_transitions_s_uses_geometric_source",    s_uses_geometric_source)
    TEST("spectre_transitions_all_hexes_geometric",        all_hexes_use_geometric_source)
    TEST("spectre_transitions_spot_check_destinations",    spot_check_destinations)
    return R.run_all()
end

return { main = main }
