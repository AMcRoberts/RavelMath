-- scripts/run_lua_tests.lua
--
-- Top-level test orchestrator. Loads the C++ backend library and
-- runs every test file under lua/tests/. Exits with a non-zero
-- status if any test fails.

local LUA_CPATH = package.cpath
package.cpath = LUA_CPATH

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."

-- Locate the C++ shared library
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath

-- Set up Lua module path so `require("ravel.*")` resolves
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

local test_files = {
    "ravel.tests.test_cyclotomic",
    "ravel.tests.test_hex",
    "ravel.tests.test_transitions",
    "ravel.tests.test_transitions_audit",
    "ravel.tests.test_spectre_transitions",
    "ravel.tests.test_coordinate",
    "ravel.tests.test_tribonacci",
    "ravel.tests.test_closure",
    "ravel.tests.test_pisot",
    "ravel.tests.test_pisot_survey",
    "ravel.tests.test_reference_sigma",
    "ravel.tests.test_predict_dimension",
    "ravel.tests.test_gap_labels",
    "ravel.tests.test_contact_boundary",
    "ravel.tests.test_general_algebraic",
    "ravel.tests.test_return_phase",
    "ravel.tests.test_bp_dump_analysis",
    "ravel.tests.test_fibonacci_selection",
    "ravel.tests.test_rauzy_fractal",
    "ravel.tests.test_wide_survey",
}

local failed = 0
for _, modname in ipairs(test_files) do
    print("================================================================")
    print("Suite: " .. modname)
    print("================================================================")
    local mod = require(modname)
    local ok, err = pcall(mod.main, ravel)
    if not ok then
        print(string.format("Suite %s failed to run: %s", modname, tostring(err)))
        failed = failed + 1
    end
    print("")
end

if failed > 0 then
    print(string.format("FAILED %d suite(s).", failed))
    os.exit(1)
end
print("ALL SUITES PASSED.")
