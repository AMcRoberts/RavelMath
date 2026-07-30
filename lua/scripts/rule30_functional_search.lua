-- Packed native replacement for cost_min2.py's dense nf-by-N search.
local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path
local ravel = require("ravel").init(require("spectre_native"))

local n = tonumber(arg and arg[1]) or 2^21
local max_sites = tonumber(arg and arg[2]) or n
local separation = 46368
local funcs, names = {}, {}
for o = 1, 12 do
    funcs[#funcs + 1] = {0, o}
    names[#names + 1] = string.format("x2(%d)", o)
end
for i = 1, 8 do
    for j = i + 1, 8 do
        funcs[#funcs + 1] = {0, i, j}
        names[#names + 1] = string.format("x3(%d,%d)", i, j)
    end
end

local result = ravel.fibonacci_dynamics.correlation_matrix(
    n, 16, separation, funcs, max_sites, #funcs)
local matrix = result.matrix
local patterns = {
    {1,1,1,-1}, {1,1,-1,1}, {1,-1,1,1}, {-1,1,1,1},
    {-1,-1,-1,1}, {-1,-1,1,-1}, {-1,1,-1,-1}, {1,-1,-1,-1},
}
local pairs = {}
for i = 1, #funcs do
    for j = i + 1, #funcs do pairs[#pairs + 1] = {i, j} end
end
local best
for _, ap in ipairs(pairs) do
    for _, bp in ipairs(pairs) do
        local t = {
            matrix[ap[1]][bp[1]], matrix[ap[1]][bp[2]],
            matrix[ap[2]][bp[1]], matrix[ap[2]][bp[2]],
        }
        for _, pattern in ipairs(patterns) do
            local cost = 0
            for k = 1, 4 do
                cost = cost + math.abs(pattern[k] / math.sqrt(2) - t[k]) / 2
            end
            if not best or cost < best.cost then
                best = {
                    cost=cost, a=ap, b=bp, pattern=pattern, correlators=t,
                }
            end
        end
    end
end
local p = best.pattern
local t = best.correlators
local s = p[1]*t[1] + p[2]*t[2] + p[3]*t[3] + p[4]*t[4]
print(string.format("packed Rule30 functional search: N=%d nf=%d", n, #funcs))
print(string.format("A={%s,%s} B={%s,%s}",
    names[best.a[1]], names[best.a[2]],
    names[best.b[1]], names[best.b[2]]))
print(string.format("pattern={%d,%d,%d,%d}", table.unpack(p)))
print(string.format("T={%.12f,%.12f,%.12f,%.12f}", table.unpack(t)))
print(string.format("S_unbiased=%.12f total_TV_floor=%.12f",
    s, (2*math.sqrt(2)-s)/2))

local selected = {
    funcs[best.a[1]], funcs[best.a[2]],
    funcs[best.b[1]], funcs[best.b[2]],
}
local targets = {}
for i = 1, 4 do targets[i] = p[i] / math.sqrt(2) end
local retarget = ravel.fibonacci_dynamics.retarget(
    n, 16, separation, 2584, 1597, selected, targets, t, p, max_sites)
local tv = 0
for i = 1, 4 do
    tv = tv + math.abs(retarget.correlators[i] - t[i]) / 2
end
print(string.format("retargeted_S=%.12f total_TV=%.12f",
    retarget.signed_chsh, tv))
