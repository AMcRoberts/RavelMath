-- Native audit of the historical Fibonacci selection theorem scripts.
--
-- This driver reports exact infinite-chain piecewise-measure evaluations.
-- It does not call a numerical observation a theorem: the historical
-- theorem_final_results.json contains counterexamples to both blanket
-- even-stride exactness and the claimed quadratic shift law.

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

-- Python's historical FIB[0] is Lua's fib[1].
local fib = {
    1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610,
    987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368,
    75025, 121393, 196418, 317811, 514229,
}
local alpha = (math.sqrt(5) - 1) / 2
local tsirelson = 2 * math.sqrt(2)

local function linear_slope(points)
    local sx, sy, sxx, sxy = 0, 0, 0, 0
    for _, p in ipairs(points) do
        sx, sy = sx + p[1], sy + p[2]
        sxx, sxy = sxx + p[1] * p[1], sxy + p[1] * p[2]
    end
    local n = #points
    return (n * sxy - sx * sy) / (n * sxx - sx * sx)
end

print("native exact infinite-chain Fibonacci selection audit")
print("(historical zero-shift fixtures; even d2 is not sufficient for exactness)")
for _, p2_python in ipairs({10, 13, 16}) do
    local d2 = fib[p2_python + 1]
    local d1 = fib[p2_python + 7]
    local r = ravel.fibonacci_selection.exact_quantum(d1, d2, 0.0)
    local max_error = 0
    local targets = {
        1 / math.sqrt(2), 1 / math.sqrt(2),
        1 / math.sqrt(2), -1 / math.sqrt(2),
    }
    for i = 1, 4 do
        max_error = math.max(max_error, math.abs(r.correlators[i] - targets[i]))
    end
    print(string.format(
        "  p2=%d d2=%d CHSH gap=%.9e max correlator gap=%.9e cells=%d",
        p2_python, d2, math.abs(r.chsh - tsirelson),
        max_error, r.partition_cells))
end

print("(shift scan relative to the computed zero-shift reference)")
local d2, d1 = fib[17], fib[23] -- historical FIB[16], FIB[22]
local reference = ravel.fibonacci_selection.exact_quantum(d1, d2, 0.0)
local points = {}
for _, m_python in ipairs({6, 8, 10, 12, 14, 16, 18, 20}) do
    local fm = fib[m_python + 1]
    local delta = math.abs(fm * alpha - math.floor(fm * alpha + 0.5))
    local r = ravel.fibonacci_selection.exact_quantum(d1, d2, delta)
    local residual = math.abs(r.chsh - reference.chsh)
    points[#points + 1] = {
        math.log(delta), math.log(math.max(residual, 1e-16)),
    }
    print(string.format(
        "  m=%d delta=%.4e |S(delta)-S(0)|=%.4e ratio/delta^2=%.4g",
        m_python, delta, residual, residual / (delta * delta)))
end
local slope = linear_slope(points)
print(string.format("  fitted log-log slope: %.6f", slope))
if math.abs(slope - 2) > 0.25 then
    print("  verdict: this scan does not support the claimed quadratic law")
else
    print("  verdict: this scan is numerically compatible with slope two")
end
