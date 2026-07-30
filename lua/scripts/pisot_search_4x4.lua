-- Native replacement for pisot_search_4x4.py.
-- C++ performs primitive-matrix generation and Pisot classification;
-- Lua selects the exact determinant class and formats results.

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

local target = tonumber(arg[1]) or 40
local entry_max = tonumber(arg[2]) or 3
local seed = tonumber(arg[3]) or 11
local candidates = ravel.survey.wide_random(
    target, entry_max, seed, 8000, 60000, 4)

local matches = {}
for _, candidate in ipairs(candidates) do
    if candidate.abs_det == 2 then
        matches[#matches + 1] = candidate
    end
end

print(string.format(
    "native 4x4 survey: %d Pisot candidates; %d with |det|=2",
    #candidates, #matches))
for _, candidate in ipairs(matches) do
    print(string.format("%s beta=%.9f b2=%.9f |det|=%d",
        candidate.name, candidate.beta, candidate.b2, candidate.abs_det))
    for _, row in ipairs(candidate.matrix) do
        print("  " .. table.concat(row, " "))
    end
end
