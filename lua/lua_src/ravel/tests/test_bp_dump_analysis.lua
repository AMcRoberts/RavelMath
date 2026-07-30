local runner = require("ravel.tests.runner").make_runner()
local TEST, run_all = runner.TEST, runner.run_all

local fixture = [[
# n=3 recurrent_core_size=2
STATES
S 0 01 10 2
S 1 0 1 1
EDGES
E 0 0 1 4 2
E 0 1 2 4 1
E 0 0 3 4 2
E 1 0 0 1 2
]]

local M = {}

function M.main(ravel)
    TEST("bp_dump_sparse_analysis", function()
        local r = ravel.balanced_pair.analyze_dump_text(fixture)
        assert(r.n == 3 and r.core_size == 2)
        assert(r.edge_records == 4 and r.extra_rows == 1)
        assert(r.correction_nilpotent and r.nilpotency_index == 1)
        assert(r.correction_lower_triangular_by_length)
        assert(r.chunk_counts_are_powers_of_two)
        assert(r.position_zero_is_transient)
        assert(r.designated_is_max_v2_position)
    end)

    TEST("bp_dump_caps_surface_as_error", function()
        local ok = pcall(function()
            ravel.balanced_pair.analyze_dump_text(
                fixture, 1048576, 100, 2)
        end)
        assert(not ok)
    end)

    assert(run_all())
end

return M
