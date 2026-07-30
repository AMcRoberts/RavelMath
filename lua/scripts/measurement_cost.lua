-- Native finite-chain measurement-dependence cost audit.
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
local separation, d1, d2 = 46368, 2584, 1597
local unbiased = ravel.fibonacci_finite.run(
    n, separation, d1, d2, nil, "polarizer", 0, nil, max_sites)

print(string.format("polarizer measurement-cost scan: N=%d", n))
for _, sigma in ipairs({2.0, 2.4, 2*math.sqrt(2), 3.2, 3.6, 3.99}) do
    local targets = {sigma/4, sigma/4, sigma/4, -sigma/4}
    local r = ravel.fibonacci_finite.run(
        n, separation, d1, d2, targets, "polarizer", 0, nil, max_sites)
    local tv = 0
    for i = 1, 4 do
        tv = tv + math.abs(r.correlators[i] - unbiased.correlators[i]) / 8
    end
    print(string.format(
        "sigma=%.12f S=%.12f mean_TV=%.12f", sigma, r.chsh, tv))
end

local q = 1 / math.sqrt(2)
local triangle = {0.5, 0.5, 0.5, -0.5}
local quantum = {q, q, q, -q}
local kl_sum = 0
for i = 1, 4 do
    local pm, pa = (1 + triangle[i])/2, (1 + quantum[i])/2
    local kl = pa*math.log(pa/pm) + (1-pa)*math.log((1-pa)/(1-pm))
    kl_sum = kl_sum + kl / math.log(2)
end
print(string.format("mean_KL_bits=%.15f", kl_sum / 4))
print("Run rule30_functional_search.lua for the packed dynamical optimum.")
