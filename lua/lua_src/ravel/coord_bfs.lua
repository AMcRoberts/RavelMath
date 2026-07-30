-- lua/ravel/coord_bfs.lua
--
-- BFS walker over the hex outer transition table. Produces a set of
-- SpectreKey = {hex, child_index, hex_edge} triples reachable from a
-- root, mirroring src/spectre_coordinate_main.cpp and the
-- `spectre_coordinate 4` audit.

local M = {}

local function queue_new() return { first = 1, last = 0 } end

local function queue_push(q, v)
    q.last = q.last + 1
    q[q.last] = v
end

local function queue_pop(q)
    if q.first > q.last then return nil end
    local v = q[q.first]
    q[q.first] = nil
    q.first = q.first + 1
    return v
end

local function queue_empty(q) return q.first > q.last end

local function key_eq(a, b)
    return a.hex == b.hex
   and a.child_index == b.child_index
   and a.hex_edge == b.hex_edge
end

local function seen_insert(seen, key)
    for _, k in ipairs(seen) do
        if key_eq(k, key) then return false end
    end
    seen[#seen + 1] = key
    return true
end

-- BFS walk starting from root_key {hex, child_index, hex_edge}.
-- Returns { generated, unique_keys, self_loops, seen }.
function M.walk(outer_table, root_key, max_steps)
    max_steps = max_steps or 4096
    local frontier = queue_new()
    local seen = {}
    queue_push(frontier, root_key)
    seen_insert(seen, root_key)
    local generated = 0
    local self_loops = 0
    while not queue_empty(frontier) do
        local cur = queue_pop(frontier)
        generated = generated + 1
        for edge = 0, 5 do
            for seg = 0, 5 do
                local t = outer_table.lookup(cur.hex, edge, seg)
                if t then
                    if t.neighbour == cur.hex then
                        self_loops = self_loops + 1
                    end
                    local next_key = {
                        hex         = t.neighbour,
                        child_index = t.child_index,
                        hex_edge    = edge,
                    }
                    if seen_insert(seen, next_key) then
                        queue_push(frontier, next_key)
                    end
                end
            end
        end
        if generated >= max_steps then break end
    end
    return {
        generated   = generated,
        unique_keys = #seen,
        self_loops  = self_loops,
        seen        = seen,
    }
end

return M
