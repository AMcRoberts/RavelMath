-- lua/ravel/lineage.lua
--
-- Parent-tree lineage for the Module hierarchy. Mirrors
-- include/ravel/spectre_lineage.hpp.
--
-- A module is a Lua table with fields { id, level, parent, children,
-- child_slot, hand, cluster_kind, cluster_id, state, weight, anchor }.
-- The BFS in lua/ravel/coord_bfs.lua produces the modules.

local M = {}

function M.lineage_of(module)
    local steps = {}
    local current = module
    while current and current.parent do
        steps[#steps + 1] = {
            module_id    = current.id,
            child_slot   = current.child_slot,
            hand         = current.hand,
            cluster_kind = current.cluster_kind,
        }
        current = current.parent
    end
    -- Reverse so the result is root-to-leaf.
    local out = {}
    for i = #steps, 1, -1 do out[#out + 1] = steps[i] end
    return out
end

function M.common_ancestor_level(first, second)
    local a, b = first, second
    while a.level > b.level do a = a.parent end
    while b.level > a.level do b = b.parent end
    while a ~= b do
        a = a.parent
        b = b.parent
    end
    return (a and a.level) or 0
end

function M.tree_distance(first, second)
    local lca = M.common_ancestor_level(first, second)
    return first.level + second.level - 2 * lca
end

return M
