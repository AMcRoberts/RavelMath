-- lua/data/hex_outer_table.lua
--
-- The 9x36 hex outer transition table transcribed from
-- include/ravel/spectre_transitions.hpp. Each row is indexed by
-- (parent_hex, parent_edge, segment) -> { neighbour, child_index }.
--
-- The values come from the Tatham hexmap_<letter>.svg labels,
-- reproduced verbatim from the C++ fill() block. Per-edge ordering
-- is by angle-from-parent-centroid (CCW). The audit test
-- (lua/tests/test_transitions_audit.lua) enforces this data against
-- the C++ source-of-truth.
--
-- Source: include/ravel/spectre_transitions.hpp (C++ project);
-- provenance in docs/THEOREM_STATUS.md "Per-edge transition
-- table values".

local hex = require("ravel.data.hex_kind")

local M = {}

-- Edge data for each hex. Inner index: edge*6 + segment (0..35).
-- Each entry is { neighbour_index, child_index }.
local RAW = {}

local function fill(k, edge, seg, nbr, child)
    RAW[k] = RAW[k] or {}
    RAW[k][edge * 6 + seg] = { nbr, child }
end

-- G
fill(hex.G, 0, 0, hex.F, 0); fill(hex.G, 0, 1, hex.F, 0); fill(hex.G, 0, 2, hex.F, 0)
fill(hex.G, 0, 3, hex.F, 0); fill(hex.G, 0, 4, hex.F, 0); fill(hex.G, 0, 5, hex.F, 0)
fill(hex.G, 1, 0, hex.X, 1); fill(hex.G, 1, 1, hex.X, 1); fill(hex.G, 1, 2, hex.X, 1)
fill(hex.G, 1, 3, hex.X, 1); fill(hex.G, 1, 4, hex.X, 1); fill(hex.G, 1, 5, hex.X, 1)
fill(hex.G, 2, 0, hex.S, 3); fill(hex.G, 2, 1, hex.S, 3); fill(hex.G, 2, 2, hex.S, 3)
fill(hex.G, 2, 3, hex.S, 3); fill(hex.G, 2, 4, hex.S, 3); fill(hex.G, 2, 5, hex.S, 3)
fill(hex.G, 3, 0, hex.J, 6); fill(hex.G, 3, 1, hex.J, 6); fill(hex.G, 3, 2, hex.J, 6)
fill(hex.G, 3, 3, hex.J, 6); fill(hex.G, 3, 4, hex.J, 6); fill(hex.G, 3, 5, hex.J, 6)
fill(hex.G, 4, 0, hex.P, 4); fill(hex.G, 4, 1, hex.P, 4); fill(hex.G, 4, 2, hex.P, 4)
fill(hex.G, 4, 3, hex.P, 4); fill(hex.G, 4, 4, hex.P, 4); fill(hex.G, 4, 5, hex.P, 4)
fill(hex.G, 5, 0, hex.G, 2); fill(hex.G, 5, 1, hex.G, 2); fill(hex.G, 5, 2, hex.G, 2)
fill(hex.G, 5, 3, hex.G, 2); fill(hex.G, 5, 4, hex.G, 2); fill(hex.G, 5, 5, hex.G, 2)

-- D
fill(hex.D, 0, 0, hex.G, 2); fill(hex.D, 0, 1, hex.G, 2); fill(hex.D, 0, 2, hex.G, 2)
fill(hex.D, 0, 3, hex.G, 2); fill(hex.D, 0, 4, hex.G, 2); fill(hex.D, 0, 5, hex.G, 2)
fill(hex.D, 1, 0, hex.F, 0); fill(hex.D, 1, 1, hex.F, 0); fill(hex.D, 1, 2, hex.F, 0)
fill(hex.D, 1, 3, hex.F, 0); fill(hex.D, 1, 4, hex.F, 0); fill(hex.D, 1, 5, hex.F, 0)
fill(hex.D, 2, 0, hex.P, 1); fill(hex.D, 2, 1, hex.P, 1); fill(hex.D, 2, 2, hex.P, 1)
fill(hex.D, 2, 3, hex.P, 1); fill(hex.D, 2, 4, hex.P, 1); fill(hex.D, 2, 5, hex.P, 1)
fill(hex.D, 3, 0, hex.S, 3); fill(hex.D, 3, 1, hex.S, 3); fill(hex.D, 3, 2, hex.S, 3)
fill(hex.D, 3, 3, hex.S, 3); fill(hex.D, 3, 4, hex.S, 3); fill(hex.D, 3, 5, hex.S, 3)
fill(hex.D, 4, 0, hex.F, 6); fill(hex.D, 4, 1, hex.F, 6); fill(hex.D, 4, 2, hex.F, 6)
fill(hex.D, 4, 3, hex.F, 6); fill(hex.D, 4, 4, hex.F, 6); fill(hex.D, 4, 5, hex.F, 6)
fill(hex.D, 5, 0, hex.X, 7); fill(hex.D, 5, 1, hex.X, 7); fill(hex.D, 5, 2, hex.X, 7)
fill(hex.D, 5, 3, hex.X, 7); fill(hex.D, 5, 4, hex.X, 7); fill(hex.D, 5, 5, hex.X, 7)

-- J
fill(hex.J, 0, 0, hex.G, 2); fill(hex.J, 0, 1, hex.G, 2); fill(hex.J, 0, 2, hex.G, 2)
fill(hex.J, 0, 3, hex.G, 2); fill(hex.J, 0, 4, hex.G, 2); fill(hex.J, 0, 5, hex.G, 2)
fill(hex.J, 1, 0, hex.F, 0); fill(hex.J, 1, 1, hex.F, 0); fill(hex.J, 1, 2, hex.F, 0)
fill(hex.J, 1, 3, hex.F, 0); fill(hex.J, 1, 4, hex.F, 0); fill(hex.J, 1, 5, hex.F, 0)
fill(hex.J, 2, 0, hex.P, 1); fill(hex.J, 2, 1, hex.P, 1); fill(hex.J, 2, 2, hex.P, 1)
fill(hex.J, 2, 3, hex.P, 1); fill(hex.J, 2, 4, hex.P, 1); fill(hex.J, 2, 5, hex.P, 1)
fill(hex.J, 3, 0, hex.S, 3); fill(hex.J, 3, 1, hex.S, 3); fill(hex.J, 3, 2, hex.S, 3)
fill(hex.J, 3, 3, hex.S, 3); fill(hex.J, 3, 4, hex.S, 3); fill(hex.J, 3, 5, hex.S, 3)
fill(hex.J, 4, 0, hex.F, 6); fill(hex.J, 4, 1, hex.F, 6); fill(hex.J, 4, 2, hex.F, 6)
fill(hex.J, 4, 3, hex.F, 6); fill(hex.J, 4, 4, hex.F, 6); fill(hex.J, 4, 5, hex.F, 6)
fill(hex.J, 5, 0, hex.Y, 4); fill(hex.J, 5, 1, hex.Y, 4); fill(hex.J, 5, 2, hex.Y, 4)
fill(hex.J, 5, 3, hex.Y, 4); fill(hex.J, 5, 4, hex.Y, 4); fill(hex.J, 5, 5, hex.Y, 4)

-- L
fill(hex.L, 0, 0, hex.G, 2); fill(hex.L, 0, 1, hex.G, 2); fill(hex.L, 0, 2, hex.G, 2)
fill(hex.L, 0, 3, hex.G, 2); fill(hex.L, 0, 4, hex.G, 2); fill(hex.L, 0, 5, hex.G, 2)
fill(hex.L, 1, 0, hex.F, 0); fill(hex.L, 1, 1, hex.F, 0); fill(hex.L, 1, 2, hex.F, 0)
fill(hex.L, 1, 3, hex.F, 0); fill(hex.L, 1, 4, hex.F, 0); fill(hex.L, 1, 5, hex.F, 0)
fill(hex.L, 2, 0, hex.P, 1); fill(hex.L, 2, 1, hex.P, 1); fill(hex.L, 2, 2, hex.P, 1)
fill(hex.L, 2, 3, hex.P, 1); fill(hex.L, 2, 4, hex.P, 1); fill(hex.L, 2, 5, hex.P, 1)
fill(hex.L, 3, 0, hex.S, 3); fill(hex.L, 3, 1, hex.S, 3); fill(hex.L, 3, 2, hex.S, 3)
fill(hex.L, 3, 3, hex.S, 3); fill(hex.L, 3, 4, hex.S, 3); fill(hex.L, 3, 5, hex.S, 3)
fill(hex.L, 4, 0, hex.F, 6); fill(hex.L, 4, 1, hex.F, 6); fill(hex.L, 4, 2, hex.F, 6)
fill(hex.L, 4, 3, hex.F, 6); fill(hex.L, 4, 4, hex.F, 6); fill(hex.L, 4, 5, hex.F, 6)
fill(hex.L, 5, 0, hex.X, 7); fill(hex.L, 5, 1, hex.X, 7); fill(hex.L, 5, 2, hex.X, 7)
fill(hex.L, 5, 3, hex.X, 7); fill(hex.L, 5, 4, hex.X, 7); fill(hex.L, 5, 5, hex.X, 7)

-- X
fill(hex.X, 0, 0, hex.G, 2); fill(hex.X, 0, 1, hex.G, 2); fill(hex.X, 0, 2, hex.G, 2)
fill(hex.X, 0, 3, hex.G, 2); fill(hex.X, 0, 4, hex.G, 2); fill(hex.X, 0, 5, hex.G, 2)
fill(hex.X, 1, 0, hex.F, 0); fill(hex.X, 1, 1, hex.F, 0); fill(hex.X, 1, 2, hex.F, 0)
fill(hex.X, 1, 3, hex.F, 0); fill(hex.X, 1, 4, hex.F, 0); fill(hex.X, 1, 5, hex.F, 0)
fill(hex.X, 2, 0, hex.Y, 1); fill(hex.X, 2, 1, hex.Y, 1); fill(hex.X, 2, 2, hex.Y, 1)
fill(hex.X, 2, 3, hex.Y, 1); fill(hex.X, 2, 4, hex.Y, 1); fill(hex.X, 2, 5, hex.Y, 1)
fill(hex.X, 3, 0, hex.S, 3); fill(hex.X, 3, 1, hex.S, 3); fill(hex.X, 3, 2, hex.S, 3)
fill(hex.X, 3, 3, hex.S, 3); fill(hex.X, 3, 4, hex.S, 3); fill(hex.X, 3, 5, hex.S, 3)
fill(hex.X, 4, 0, hex.F, 6); fill(hex.X, 4, 1, hex.F, 6); fill(hex.X, 4, 2, hex.F, 6)
fill(hex.X, 4, 3, hex.F, 6); fill(hex.X, 4, 4, hex.F, 6); fill(hex.X, 4, 5, hex.F, 6)
fill(hex.X, 5, 0, hex.P, 7); fill(hex.X, 5, 1, hex.P, 7); fill(hex.X, 5, 2, hex.P, 7)
fill(hex.X, 5, 3, hex.P, 7); fill(hex.X, 5, 4, hex.P, 7); fill(hex.X, 5, 5, hex.P, 7)

-- P
fill(hex.P, 0, 0, hex.G, 2); fill(hex.P, 0, 1, hex.G, 2); fill(hex.P, 0, 2, hex.G, 2)
fill(hex.P, 0, 3, hex.G, 2); fill(hex.P, 0, 4, hex.G, 2); fill(hex.P, 0, 5, hex.G, 2)
fill(hex.P, 1, 0, hex.F, 0); fill(hex.P, 1, 1, hex.F, 0); fill(hex.P, 1, 2, hex.F, 0)
fill(hex.P, 1, 3, hex.F, 0); fill(hex.P, 1, 4, hex.F, 0); fill(hex.P, 1, 5, hex.F, 0)
fill(hex.P, 2, 0, hex.Y, 1); fill(hex.P, 2, 1, hex.Y, 1); fill(hex.P, 2, 2, hex.Y, 1)
fill(hex.P, 2, 3, hex.Y, 1); fill(hex.P, 2, 4, hex.Y, 1); fill(hex.P, 2, 5, hex.Y, 1)
fill(hex.P, 3, 0, hex.S, 3); fill(hex.P, 3, 1, hex.S, 3); fill(hex.P, 3, 2, hex.S, 3)
fill(hex.P, 3, 3, hex.S, 3); fill(hex.P, 3, 4, hex.S, 3); fill(hex.P, 3, 5, hex.S, 3)
fill(hex.P, 4, 0, hex.F, 6); fill(hex.P, 4, 1, hex.F, 6); fill(hex.P, 4, 2, hex.F, 6)
fill(hex.P, 4, 3, hex.F, 6); fill(hex.P, 4, 4, hex.F, 6); fill(hex.P, 4, 5, hex.F, 6)
fill(hex.P, 5, 0, hex.X, 7); fill(hex.P, 5, 1, hex.X, 7); fill(hex.P, 5, 2, hex.X, 7)
fill(hex.P, 5, 3, hex.X, 7); fill(hex.P, 5, 4, hex.X, 7); fill(hex.P, 5, 5, hex.X, 7)

-- S
fill(hex.S, 0, 0, hex.G, 2); fill(hex.S, 0, 1, hex.G, 2); fill(hex.S, 0, 2, hex.G, 2)
fill(hex.S, 0, 3, hex.G, 2); fill(hex.S, 0, 4, hex.G, 2); fill(hex.S, 0, 5, hex.G, 2)
fill(hex.S, 1, 0, hex.L, 0); fill(hex.S, 1, 1, hex.L, 0); fill(hex.S, 1, 2, hex.L, 0)
fill(hex.S, 1, 3, hex.L, 0); fill(hex.S, 1, 4, hex.L, 0); fill(hex.S, 1, 5, hex.L, 0)
fill(hex.S, 2, 0, hex.P, 1); fill(hex.S, 2, 1, hex.P, 1); fill(hex.S, 2, 2, hex.P, 1)
fill(hex.S, 2, 3, hex.P, 1); fill(hex.S, 2, 4, hex.P, 1); fill(hex.S, 2, 5, hex.P, 1)
fill(hex.S, 3, 0, hex.F, 6); fill(hex.S, 3, 1, hex.F, 6); fill(hex.S, 3, 2, hex.F, 6)
fill(hex.S, 3, 3, hex.F, 6); fill(hex.S, 3, 4, hex.F, 6); fill(hex.S, 3, 5, hex.F, 6)
fill(hex.S, 4, 0, hex.X, 7); fill(hex.S, 4, 1, hex.X, 7); fill(hex.S, 4, 2, hex.X, 7)
fill(hex.S, 4, 3, hex.X, 7); fill(hex.S, 4, 4, hex.X, 7); fill(hex.S, 4, 5, hex.X, 7)
fill(hex.S, 5, 0, hex.X, 4); fill(hex.S, 5, 1, hex.X, 4); fill(hex.S, 5, 2, hex.X, 4)
fill(hex.S, 5, 3, hex.X, 4); fill(hex.S, 5, 4, hex.X, 4); fill(hex.S, 5, 5, hex.X, 4)

-- F
fill(hex.F, 0, 0, hex.G, 2); fill(hex.F, 0, 1, hex.G, 2); fill(hex.F, 0, 2, hex.G, 2)
fill(hex.F, 0, 3, hex.G, 2); fill(hex.F, 0, 4, hex.G, 2); fill(hex.F, 0, 5, hex.G, 2)
fill(hex.F, 1, 0, hex.F, 0); fill(hex.F, 1, 1, hex.F, 0); fill(hex.F, 1, 2, hex.F, 0)
fill(hex.F, 1, 3, hex.F, 0); fill(hex.F, 1, 4, hex.F, 0); fill(hex.F, 1, 5, hex.F, 0)
fill(hex.F, 2, 0, hex.P, 1); fill(hex.F, 2, 1, hex.P, 1); fill(hex.F, 2, 2, hex.P, 1)
fill(hex.F, 2, 3, hex.P, 1); fill(hex.F, 2, 4, hex.P, 1); fill(hex.F, 2, 5, hex.P, 1)
fill(hex.F, 3, 0, hex.S, 3); fill(hex.F, 3, 1, hex.S, 3); fill(hex.F, 3, 2, hex.S, 3)
fill(hex.F, 3, 3, hex.S, 3); fill(hex.F, 3, 4, hex.S, 3); fill(hex.F, 3, 5, hex.S, 3)
fill(hex.F, 4, 0, hex.Y, 7); fill(hex.F, 4, 1, hex.Y, 7); fill(hex.F, 4, 2, hex.Y, 7)
fill(hex.F, 4, 3, hex.Y, 7); fill(hex.F, 4, 4, hex.Y, 7); fill(hex.F, 4, 5, hex.Y, 7)
fill(hex.F, 5, 0, hex.Y, 4); fill(hex.F, 5, 1, hex.Y, 4); fill(hex.F, 5, 2, hex.Y, 4)
fill(hex.F, 5, 3, hex.Y, 4); fill(hex.F, 5, 4, hex.Y, 4); fill(hex.F, 5, 5, hex.Y, 4)

-- Y
fill(hex.Y, 0, 0, hex.G, 2); fill(hex.Y, 0, 1, hex.G, 2); fill(hex.Y, 0, 2, hex.G, 2)
fill(hex.Y, 0, 3, hex.G, 2); fill(hex.Y, 0, 4, hex.G, 2); fill(hex.Y, 0, 5, hex.G, 2)
fill(hex.Y, 1, 0, hex.F, 0); fill(hex.Y, 1, 1, hex.F, 0); fill(hex.Y, 1, 2, hex.F, 0)
fill(hex.Y, 1, 3, hex.F, 0); fill(hex.Y, 1, 4, hex.F, 0); fill(hex.Y, 1, 5, hex.F, 0)
fill(hex.Y, 2, 0, hex.Y, 1); fill(hex.Y, 2, 1, hex.Y, 1); fill(hex.Y, 2, 2, hex.Y, 1)
fill(hex.Y, 2, 3, hex.Y, 1); fill(hex.Y, 2, 4, hex.Y, 1); fill(hex.Y, 2, 5, hex.Y, 1)
fill(hex.Y, 3, 0, hex.S, 3); fill(hex.Y, 3, 1, hex.S, 3); fill(hex.Y, 3, 2, hex.S, 3)
fill(hex.Y, 3, 3, hex.S, 3); fill(hex.Y, 3, 4, hex.S, 3); fill(hex.Y, 3, 5, hex.S, 3)
fill(hex.Y, 4, 0, hex.F, 6); fill(hex.Y, 4, 1, hex.F, 6); fill(hex.Y, 4, 2, hex.F, 6)
fill(hex.Y, 4, 3, hex.F, 6); fill(hex.Y, 4, 4, hex.F, 6); fill(hex.Y, 4, 5, hex.F, 6)
fill(hex.Y, 5, 0, hex.Y, 4); fill(hex.Y, 5, 1, hex.Y, 4); fill(hex.Y, 5, 2, hex.Y, 4)
fill(hex.Y, 5, 3, hex.Y, 4); fill(hex.Y, 5, 4, hex.Y, 4); fill(hex.Y, 5, 5, hex.Y, 4)

function M.lookup(kind, edge, segment)
    if kind == hex.COUNT then return nil end
    if edge < 0 or edge >= 6 then return nil end
    if segment < 0 or segment >= 6 then return nil end
    local row = RAW[kind]
    if not row then return nil end
    local v = row[edge * 6 + segment]
    if not v then return nil end
    if v[1] == hex.COUNT then return nil end
    return { neighbour = v[1], child_index = v[2] }
end

function M.table_complete()
    for _, k in ipairs(hex.all_kinds()) do
        for edge = 0, 5 do
            for seg = 0, 5 do
                if M.lookup(k, edge, seg) == nil then
                    return false
                end
            end
        end
    end
    return true
end

return M
