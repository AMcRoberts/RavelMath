-- scripts/coord_bfs.lua
--
-- Standalone script that mirrors src/spectre_coordinate_main.cpp in
-- the C++ project. Walks the hex outer transition table from a root
-- key and reports generated / unique / self-loop counts plus a per-hex
-- distribution. Used by the verification recipe.

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."

package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

local radius = tonumber(arg[1]) or 4

io.write(string.format("Spectre coordinate audit (radius %d)\n", radius))
local root = { hex = ravel.hex_kind.G, child_index = 0, hex_edge = 0 }
local result = ravel.coord_bfs.walk(ravel.outer, root, 4096)
io.write(string.format("  generated=%d unique_keys=%d self_loops=%d\n",
    result.generated, result.unique_keys, result.self_loops))

local per_kind = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
for _, k in ipairs(result.seen) do
    per_kind[k.hex + 1] = per_kind[k.hex + 1] + 1
end
local sample = result.seen[1]
io.write(string.format("  sample hex=%d child=%d edge=%d\n",
    sample.hex, sample.child_index, sample.hex_edge))
for _, kind in ipairs(ravel.hex_kind.all_kinds()) do
    io.write(string.format("    %s: %d\n", ravel.hex_kind.name(kind), per_kind[kind + 1]))
end
