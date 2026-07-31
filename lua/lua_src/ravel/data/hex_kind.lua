-- lua/data/hex_kind.lua
--
-- HexKind enum (Tatham's Latin rebindings of the paper's Greek
-- letters). Pure-Lua original; no `include/ravel/spectre_hex.hpp`
-- exists or ever existed in this repository -- there is no C++
-- enum to stay interchangeable with.

local M = {}

M.G     = 0
M.D     = 1
M.J     = 2
M.L     = 3
M.X     = 4
M.P     = 5
M.S     = 6
M.F     = 7
M.Y     = 8
M.COUNT = 9

M.NAMES = { "G", "D", "J", "L", "X", "P", "S", "F", "Y", "?" }
M.KINDS = { 0, 1, 2, 3, 4, 5, 6, 7, 8 }

M.NAME_TO_INDEX = {}
for i, n in ipairs(M.NAMES) do
    if n ~= "?" then
        M.NAME_TO_INDEX[n] = i - 1
    end
end

local CHILD_COUNTS = {
    [M.G] = 2,
    [M.D] = 1, [M.J] = 1, [M.L] = 1,
    [M.X] = 1, [M.P] = 1, [M.S] = 1,
    [M.F] = 1, [M.Y] = 1,
}

function M.name(index)
    return M.NAMES[index + 1] or "?"
end

function M.all_kinds()
    local out = {}
    for i, k in ipairs(M.KINDS) do out[i] = k end
    return out
end

function M.child_count(kind)
    return CHILD_COUNTS[kind] or 0
end

return M
