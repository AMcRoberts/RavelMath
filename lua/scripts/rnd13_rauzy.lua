-- Correct native replacement for rnd13_rauzy.py and
-- rnd13_rauzy_ifs.py. Heavy graph-directed IFS iteration and
-- projection run in C++; this driver writes bounded TSV point data.

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

local n_points = tonumber(arg[1]) or 50000
local output_path = arg[2] or "rnd13_rauzy_points.tsv"
local seed = tonumber(arg[3]) or 42

-- Canonical column words for
-- M={{2,2,2,3},{1,0,0,0},{1,1,0,1},{2,2,2,2}}.
local rnd13 = {
    {0, 0, 1, 2, 3, 3},
    {0, 0, 2, 3, 3},
    {0, 0, 3, 3},
    {0, 0, 0, 2, 3, 3},
}

local result = ravel.rauzy_fractal.generate(rnd13, n_points, seed)
local out, err = io.open(output_path, "w")
assert(out, err)
out:write(string.format("# beta=%.17g seed=%d points=%d\n",
    result.beta, seed, n_points))
out:write("# x\ty\tz\n")
for i = 1, #result.points, 3 do
    out:write(string.format("%.17g\t%.17g\t%.17g\n",
        result.points[i], result.points[i + 1], result.points[i + 2]))
end
out:close()

print(string.format(
    "wrote %d native Rauzy points to %s (beta=%.9f, max affine error=%.3e)",
    n_points, output_path, result.beta, result.max_affine_error))
