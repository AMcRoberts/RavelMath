local runner = require("ravel.tests.runner").make_runner()
local TEST, run_all = runner.TEST, runner.run_all

local M = {}

function M.main(ravel)
    TEST("rauzy_native_generate", function()
        local tetrabonacci = {{0, 1}, {0, 2}, {0, 3}, {0}}
        local result = ravel.rauzy_fractal.generate(
            tetrabonacci, 256, 42)
        assert(result.beta > 1.9 and result.beta < 2.0)
        assert(#result.points == 256 * 3)
        assert(result.max_affine_error < 1e-9)
        for _, value in ipairs(result.points) do
            assert(value == value)
            assert(value ~= math.huge and value ~= -math.huge)
        end
    end)

    TEST("rauzy_native_caps_allocation", function()
        local tetrabonacci = {{0, 1}, {0, 2}, {0, 3}, {0}}
        local ok = pcall(function()
            ravel.rauzy_fractal.generate(tetrabonacci, 5000001, 42)
        end)
        assert(not ok)
    end)

    assert(run_all())
end

return M
