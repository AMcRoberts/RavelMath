-- lua/tests/test_transitions_audit.lua
--
-- Per-edge audit of the hex outer transition table against the C++
-- source-of-truth documented in docs/THEOREM_STATUS.md. Each
-- of the 9 hexes has 6 expected (edge, neighbour, child) triples;
-- every segment of a given edge must map to the same neighbour and
-- child. Mirrors tests/test_spectre_transitions_audit.cpp.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST
local EQ = runner.assert_eq

local outer
local hex

local function build_expected()
    local h = hex
    return {
        [h.G] = {
            { 0, h.F, 0 }, { 1, h.X, 1 }, { 2, h.S, 3 },
            { 3, h.J, 6 }, { 4, h.P, 4 }, { 5, h.G, 2 },
        },
        [h.D] = {
            { 0, h.G, 2 }, { 1, h.F, 0 }, { 2, h.P, 1 },
            { 3, h.S, 3 }, { 4, h.F, 6 }, { 5, h.X, 7 },
        },
        [h.J] = {
            { 0, h.G, 2 }, { 1, h.F, 0 }, { 2, h.P, 1 },
            { 3, h.S, 3 }, { 4, h.F, 6 }, { 5, h.Y, 4 },
        },
        [h.L] = {
            { 0, h.G, 2 }, { 1, h.F, 0 }, { 2, h.P, 1 },
            { 3, h.S, 3 }, { 4, h.F, 6 }, { 5, h.X, 7 },
        },
        [h.X] = {
            { 0, h.G, 2 }, { 1, h.F, 0 }, { 2, h.Y, 1 },
            { 3, h.S, 3 }, { 4, h.F, 6 }, { 5, h.P, 7 },
        },
        [h.P] = {
            { 0, h.G, 2 }, { 1, h.F, 0 }, { 2, h.Y, 1 },
            { 3, h.S, 3 }, { 4, h.F, 6 }, { 5, h.X, 7 },
        },
        [h.S] = {
            { 0, h.G, 2 }, { 1, h.L, 0 }, { 2, h.P, 1 },
            { 3, h.F, 6 }, { 4, h.X, 7 }, { 5, h.X, 4 },
        },
        [h.F] = {
            { 0, h.G, 2 }, { 1, h.F, 0 }, { 2, h.P, 1 },
            { 3, h.S, 3 }, { 4, h.Y, 7 }, { 5, h.Y, 4 },
        },
        [h.Y] = {
            { 0, h.G, 2 }, { 1, h.F, 0 }, { 2, h.Y, 1 },
            { 3, h.S, 3 }, { 4, h.F, 6 }, { 5, h.Y, 4 },
        },
    }
end

local function per_edge_matches_svg_audit()
    local expected = build_expected()
    for _, kind in ipairs(hex.all_kinds()) do
        local exp = expected[kind]
        for _, e in ipairs(exp) do
            local edge, want_nbr, want_child = e[1], e[2], e[3]
            for seg = 0, 5 do
                local t = outer.lookup(kind, edge, seg)
                EQ(t.neighbour,    want_nbr,   string.format("%s edge %d seg %d neighbour", hex.name(kind), edge, seg))
                EQ(t.child_index,  want_child, string.format("%s edge %d seg %d child", hex.name(kind), edge, seg))
            end
        end
    end
end

local function count_includes_g_extra_child()
    -- G has 2 children; the other 8 hexes have 1.
    EQ(hex.child_count(hex.G), 2)
    EQ(hex.child_count(hex.D), 1)
end

local function main(spectre_module)
    outer = spectre_module.outer
    hex = spectre_module.hex_kind
    TEST("transition_per_edge_matches_svg_audit", per_edge_matches_svg_audit)
    TEST("transition_count_includes_g_extra_child", count_includes_g_extra_child)
    return R.run_all()
end

return { main = main }
