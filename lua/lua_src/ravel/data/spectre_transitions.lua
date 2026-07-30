-- lua/ravel/data/spectre_transitions.lua
--
-- Per-Spectre transition table produced by
-- scripts/build_spectre_transition_table.lua. For each parent hex
-- kind L and each of its Spectre children (0 = dominant Spectre,
-- 1 = inner child, G only), this module maps the 14 Spectre edges
-- to the destination hex kind and the parent hex edge that the
-- Spectre edge sits on.
--
-- The table is the lua_reimplementation equivalent of the C++
-- `build_spectre_transition_table()` function described in
-- docs/THEOREM_STATUS.md phase 2. For G and S the values come
-- from a geometric match against the preserved specmap SVG. For
-- the other 7 hexes the values come from the closest-segment
-- matching against the txt-fallback enriched files (the SVGs are
-- not preserved in the reference folder); see the `source` field
-- in each record and the comment block in
-- scripts/build_spectre_transition_table.lua for details.
--
-- Usage:
--   local transitions = require("ravel.data.spectre_transitions")
--   local r = transitions.lookup(parent_kind, child_index, spectre_edge)
--   -- r = { parent_edge = ..., neighbour = ..., source = ... } or nil

local M = {}

local KIND_INDICES = { G = 0, D = 1, J = 2, L = 3, X = 4,
                       P = 5, S = 6, F = 7, Y = 8 }
-- KIND_NAMES is 1-indexed: KIND_NAMES[1] = "G", ..., KIND_NAMES[9] = "Y".
local KIND_NAMES  = { "G", "D", "J", "L", "X", "P", "S", "F", "Y" }

local function letter_for_kind(kind)
    return KIND_NAMES[kind + 1]
end

local function parse_record(line)
    local ci, edge, pe, nbr, src =
        line:match("^child=(%d+)%s+edge=%s*(%d+)%s+parent_edge=(%-?%d+)%s+neighbour=(%S+)%s+source=(%S+)")
    if not ci then return nil end
    local neighbour
    if nbr ~= "?" then
        neighbour = KIND_INDICES[nbr]
    end
    return {
        child_index = tonumber(ci),
        spectre_edge = tonumber(edge),
        parent_edge = tonumber(pe) >= 0 and tonumber(pe) or nil,
        neighbour = neighbour,
        source = src,
    }
end

-- Resolve the directory containing this Lua module. Uses debug info
-- for the calling frame; in Lua 5.1 the source is "@path", in 5.4
-- it is "=path" or "@path".  Falls back to CWD-relative path if
-- the file path can't be resolved.
local function module_dir()
    local info = debug.getinfo(1, "S")
    local src = info.source or ""
    src = src:gsub("^@", "")
    if src == "" or src == "=(command line)" or src:sub(1, 1) == "=" then
        return nil
    end
    return src:match("(.*/)") or "./"
end

local tables = {}
local function table_for(kind)
    if tables[kind] == nil then
        local letter = letter_for_kind(kind)
        local records = {}
        local candidates = {}
        local dir = module_dir()
        if dir then
            candidates[#candidates + 1] = string.format(
                "%sspectre_transitions/spectre_transitions_%s.txt", dir, letter)
        end
        candidates[#candidates + 1] = string.format(
            "lua_src/ravel/data/spectre_transitions/spectre_transitions_%s.txt",
            letter)
        local f
        for _, path in ipairs(candidates) do
            f = io.open(path, "r")
            if f then break end
        end
        if f then
            for line in f:lines() do
                local r = parse_record(line)
                if r then records[#records + 1] = r end
            end
            f:close()
        end
        tables[kind] = records
    end
    return tables[kind]
end

function M.lookup(kind, child_index, spectre_edge)
    for _, r in ipairs(table_for(kind)) do
        if r.child_index == child_index and r.spectre_edge == spectre_edge then
            return r
        end
    end
    return nil
end

function M.all_for(kind)
    return table_for(kind)
end

function M.summary()
    local out = {}
    for _, letter in ipairs(KIND_NAMES) do
        out[letter] = #table_for(KIND_INDICES[letter])
    end
    return out
end

return M
