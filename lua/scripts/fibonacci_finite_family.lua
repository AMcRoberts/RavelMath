-- Native replacement driver for fibmodel.py's finite polarizer/Rule-30 core.
local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local ravel = require("ravel").init(require("spectre_native"))
local n = tonumber(arg and arg[1]) or 2^20
local max_sites = tonumber(arg and arg[2]) or n
local separation, d1, d2 = 46368, 2584, 1597
local q = {
    1 / math.sqrt(2), 1 / math.sqrt(2),
    1 / math.sqrt(2), -1 / math.sqrt(2),
}

print(string.format("finite Fibonacci/Rule-30 audit: N=%d", n))
local polarizer = ravel.fibonacci_finite.run(
    n, separation, d1, d2, q, "polarizer", 0, nil, max_sites)
print(string.format("polarizer quantum retarget: S=%.12f", polarizer.chsh))

local base = ravel.fibonacci_finite.run(
    n, separation, d1, d2, nil, "rule30", 16, nil, max_sites)
print(string.format(
    "Rule30 baseline: S=%.12f E={%.9f, %.9f, %.9f, %.9f}",
    base.chsh, table.unpack(base.correlators)))

local dynamics = ravel.fibonacci_finite.run(
    n, separation, d1, d2, q, "rule30", 16,
    base.correlators, max_sites)
print(string.format(
    "Rule30 quantum retarget: S=%.12f counts={%d,%d,%d,%d}",
    dynamics.chsh, table.unpack(dynamics.accepted_count)))
