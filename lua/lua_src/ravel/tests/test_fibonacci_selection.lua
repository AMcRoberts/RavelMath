local runner = require("ravel.tests.runner").make_runner()
local TEST, run_all = runner.TEST, runner.run_all

local M = {}

function M.main(ravel)
    TEST("fibonacci_exact_quantum_even_stride", function()
        local r = ravel.fibonacci_selection.exact_quantum(
            46368, 2584, 0.0)
        assert(math.abs(r.chsh - 2 * math.sqrt(2)) < 1e-11)
        assert(#r.correlators == 4)
        assert(#r.accepted_measure == 4)
        assert(r.partition_cells <= 4 * 2584 + 8)
    end)

    TEST("fibonacci_exact_accepts_general_targets", function()
        local targets = {0.5, -0.25, 0.125, -0.75}
        local r = ravel.fibonacci_selection.exact(
            46368, 2584, 0.0, targets)
        assert(#r.correlators == 4)
        assert(#r.accepted_measure == 4)
        for i = 1, 4 do
            assert(math.abs(r.correlators[i] - targets[i]) < 1e-11)
            assert(r.accepted_measure[i] > 0)
        end
    end)

    TEST("fibonacci_exact_partition_cap", function()
        local ok = pcall(function()
            ravel.fibonacci_selection.exact_quantum(
                46368, 2584, 0.0, 100)
        end)
        assert(not ok)
    end)

    TEST("fibonacci_finite_polarizer_and_rule30", function()
        local q = {
            1 / math.sqrt(2), 1 / math.sqrt(2),
            1 / math.sqrt(2), -1 / math.sqrt(2),
        }
        local polarizer = ravel.fibonacci_finite.run(
            65536, 46368, 2584, 1597, q)
        assert(math.abs(polarizer.chsh - 2 * math.sqrt(2)) < 0.04)
        assert(polarizer.sites == 65536)

        local base = ravel.fibonacci_finite.run(
            65536, 46368, 2584, 1597, nil, "rule30", 16)
        local dynamics = ravel.fibonacci_finite.run(
            65536, 46368, 2584, 1597, q, "rule30", 16,
            base.correlators)
        assert(dynamics.chsh == dynamics.chsh)
        assert(#dynamics.accepted_count == 4)
    end)

    TEST("fibonacci_finite_site_cap", function()
        local ok = pcall(function()
            ravel.fibonacci_finite.run(
                1001, 1, 1, 1, nil, "polarizer", 0, nil, 1000)
        end)
        assert(not ok)
    end)

    TEST("fibonacci_packed_dynamics_correlations", function()
        local r = ravel.fibonacci_dynamics.correlation_matrix(
            65536, 16, 46368,
            {{0, 1}, {0, 3}, {0, 2}, {0, 5}}, 65536, 4)
        assert(r.sites == 65536)
        assert(r.function_count == 4)
        assert(#r.matrix == 4 and #r.matrix[1] == 4)
        for i = 1, 4 do
            for j = 1, 4 do
                assert(r.matrix[i][j] >= -1 and r.matrix[i][j] <= 1)
            end
        end
        local q = 1 / math.sqrt(2)
        local selected = {{0, 1}, {0, 3}, {0, 2}, {0, 5}}
        local baseline = {
            r.matrix[1][3], r.matrix[1][4],
            r.matrix[2][3], r.matrix[2][4],
        }
        local rr = ravel.fibonacci_dynamics.retarget(
            65536, 16, 46368, 2584, 1597, selected,
            {q, q, q, -q}, baseline, {1, 1, 1, -1}, 65536)
        assert(math.abs(rr.signed_chsh - 2 * math.sqrt(2)) < 0.04)
    end)

    TEST("fibonacci_local_windows", function()
        local q = 1 / math.sqrt(2)
        local r = ravel.fibonacci_dynamics.local_windows(
            65536, 8, 46368, 2584, 1597,
            {q, q, q, -q}, 65536)
        assert(math.abs(r.baseline_chsh - 2) < 1e-3)
        assert(math.abs(r.signed_chsh - 2 * math.sqrt(2)) < 0.04)
        assert(#r.baseline == 4 and #r.accepted_count == 4)
    end)

    assert(run_all())
end

return M
