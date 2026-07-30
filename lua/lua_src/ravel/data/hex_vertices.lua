-- lua/data/hex_vertices.lua
--
-- Unit hexagon vertex offsets, expressed as integer 12th cyclotomic
-- tuples {a, b, c, e} for a + b*d + c*d^2 + e*d^3. The six vertices
-- form a regular hexagon of unit circumradius, centred at the origin.
-- This matches include/ravel/spectre_coordinate.hpp::hex_vertex_offsets.
--
-- The conversion to (x, y) lives in ravel.cyclotomic.to_xy and is
-- applied at the use site so that all arithmetic in the data layer
-- stays exact in the 12th cyclotomic ring.

local M = {}

-- Hex vertices as integer-indexed tables so they round-trip cleanly
-- through the C++ cyclotomic.{make, sub, add} helpers, which use
-- lua_rawgeti(L, idx, 1..4).  Indexing mirrors the C++ struct:
--   OFFSETS[i] = { a, b, c, e } for the i-th vertex of a unit hexagon.
local function z(a, b, c, e) return { a, b, c, e } end

M.OFFSETS = {
    z( 0,  0, -1,  0),  -- 1: bottom-left
    z( 1,  0, -1,  0),  -- 2: bottom-right
    z( 1,  0,  0,  0),  -- 3: top-right
    z( 0,  0,  1,  0),  -- 4: top
    z(-1,  0,  1,  0),  -- 5: top-left
    z(-1,  0,  0,  0),  -- 6: bottom
}

function M.vertex(i)
    return M.OFFSETS[((i % 6) + 1)]
end

function M.opposite_edge(edge)
    return (edge + 3) % 6
end

return M
