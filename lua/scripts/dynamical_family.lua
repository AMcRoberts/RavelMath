-- Bounded native replacement for dynamical_v2.py and dynamical_v3.py.
-- The large binary fields live packed in C++; Lua only orchestrates
-- constant-size result records.
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
local fib = {
    1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,
    4181,6765,10946,17711,28657,46368,75025,121393,196418,
}
local alpha = (math.sqrt(5) - 1) / 2
local q = 1 / math.sqrt(2)
local targets, signs = {q,q,-q,q}, {1,1,-1,1}
local funcs = {{0,4}, {0,4,6}, {0,1,7}, {0,1,8}}

local function baseline_at(separation)
    local r = ravel.fibonacci_dynamics.correlation_matrix(
        n, 16, separation, funcs, max_sites, 4)
    return {
        r.matrix[1][3], r.matrix[1][4],
        r.matrix[2][3], r.matrix[2][4],
    }
end

local function retarget_at(separation, d1, d2, baseline)
    return ravel.fibonacci_dynamics.retarget(
        n, 16, separation, d1, d2, funcs, targets, baseline,
        signs, max_sites)
end

local base = baseline_at(fib[23])
local sunb = base[1] + base[2] - base[3] + base[4]
print(string.format(
    "optimized packed dynamical substrate: N=%d S_unbiased=%.12f",
    n, sunb))
for _, p in ipairs({11,13,15,17,19}) do
    local r = retarget_at(fib[23], fib[p + 1], fib[p], base)
    print(string.format(
        "convergence p=%d S=%.12f error=%.6g",
        p, r.signed_chsh, math.abs(r.signed_chsh - 2*math.sqrt(2))))
end

local calibration = baseline_at(fib[25])
local slope_points = {}
print("separation sweep (calibrated at m=24):")
for _, m in ipairs({6,7,8,9,10,11,12}) do
    local r = retarget_at(fib[m + 1], fib[20], fib[19], calibration)
    local reference =
        retarget_at(fib[25], fib[20], fib[19], calibration)
    local delta = math.abs(fib[m + 1] * alpha
                           - math.floor(fib[m + 1] * alpha + 0.5))
    local ds = math.abs(r.signed_chsh - reference.signed_chsh)
    if ds > 4e-3 then
        slope_points[#slope_points + 1] = {math.log(delta), math.log(ds)}
    end
    print(string.format("m=%d delta=%.6g dS=%.6g", m, delta, ds))
end
local sx, sy, sxx, sxy = 0, 0, 0, 0
for _, point in ipairs(slope_points) do
    sx, sy = sx + point[1], sy + point[2]
    sxx, sxy = sxx + point[1]^2, sxy + point[1]*point[2]
end
local count = #slope_points
local slope = count >= 2
    and (count*sxy - sx*sy) / (count*sxx - sx*sx) or 0/0
print(string.format("floor-cleared log-log slope=%.9f", slope))

print("snapped local windows:")
for _, radius in ipairs({8,16,32,64,128,256,512}) do
    local r = ravel.fibonacci_dynamics.local_windows(
        n, radius, fib[23], fib[17], fib[16],
        {q,q,q,-q}, max_sites)
    print(string.format(
        "R=%d S_unbiased=%.12f gap=%.6g",
        radius, r.baseline_chsh, 2-r.baseline_chsh))
    if radius == 8 then
        print(string.format(
            "R=8 T={%.12f,%.12f,%.12f,%.12f} retargeted_S=%.12f",
            r.baseline[1], r.baseline[2], r.baseline[3], r.baseline[4],
            r.signed_chsh))
    end
end
print("CORRECTION: the saved v2 locality fixture has zero gap at every R;")
print("it does not support the claimed Theta(1/R) law.")
