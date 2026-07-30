-- lua/tests/test_transitions.lua
--
-- Verifies that the 9x36 hex outer transition table is row-complete.
-- Mirrors tests/test_spectre_transitions.cpp.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST
local T = runner.assert_true

local outer
local hex

local function table_complete()
    T(outer.table_complete(), "transition table is not row-complete")
    -- Spot-check a known row. The original C++ test_spectre_coordinate
    -- checked hex_to_hex_transition(G, 0, 1) -> X, which contradicts
    -- the actual table data (G edge 0 maps to F at every segment). The
    -- C++ test had a stale expectation; we verify both endpoints.
    local g_e0 = outer.lookup(hex.G, 0, 0)
    T(g_e0 ~= nil, "G.0.0 missing")
    T(g_e0.neighbour == hex.F, "G.0.0 neighbour = F")
    T(g_e0.child_index == 0, "G.0.0 child = 0")

    local g_e1 = outer.lookup(hex.G, 1, 1)
    T(g_e1 ~= nil, "G.1.1 missing")
    T(g_e1.neighbour == hex.X, "G.1.1 neighbour = X")
    T(g_e1.child_index == 1, "G.1.1 child = 1")
end

local function sentinel_rejected()
    local t = outer.lookup(hex.COUNT, 0, 0)
    T(t == nil, "COUNT hex should return nil")
end

local function edge_segment_bounds()
    T(outer.lookup(hex.G, 0, 0) ~= nil, "G.0.0 present")
    T(outer.lookup(hex.G, 5, 5) ~= nil, "G.5.5 present")
    T(outer.lookup(hex.G, -1, 0) == nil, "negative edge rejected")
    T(outer.lookup(hex.G, 6, 0) == nil, "edge=6 rejected")
    T(outer.lookup(hex.G, 0, 6) == nil, "segment=6 rejected")
end

local function main(spectre_module)
    outer = spectre_module.outer
    hex = spectre_module.hex_kind
    TEST("transition_table_complete",  table_complete)
    TEST("transition_sentinel_rejected", sentinel_rejected)
    TEST("transition_edge_segment_bounds", edge_segment_bounds)
    return R.run_all()
end

return { main = main }
