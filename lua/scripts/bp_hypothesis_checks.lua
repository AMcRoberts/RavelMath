-- Native sparse replacement for bp_analysis.py and
-- bp_hypothesis_checks.py. Dumps are produced once by
-- app/bp_dump_provenance.cpp; all analysis runs in bounded C++.

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

if not arg[1] then
    io.stderr:write("usage: lua5.4 lua/scripts/bp_hypothesis_checks.lua DUMP...\n")
    os.exit(2)
end

local failed = false
for _, path in ipairs(arg) do
    local ok, r = pcall(function()
        return ravel.balanced_pair.analyze_dump(
            path, 64 * 1024 * 1024, 100000, 2000000)
    end)
    if not ok then
        failed = true
        io.stderr:write(path .. ": ERROR: " .. tostring(r) .. "\n")
    else
        print(string.format(
            "%s: n=%d N=%d m=%d edges=%d nilpotent=%s(index=%d) " ..
            "lower_by_length=%s chunks_pow2=%s pos0_transient=%s " ..
            "designated_max_v2=%s",
            path, r.n, r.core_size, r.extra_rows, r.edge_records,
            tostring(r.correction_nilpotent), r.nilpotency_index,
            tostring(r.correction_lower_triangular_by_length),
            tostring(r.chunk_counts_are_powers_of_two),
            tostring(r.position_zero_is_transient),
            tostring(r.designated_is_max_v2_position)))
    end
end

if failed then os.exit(1) end
