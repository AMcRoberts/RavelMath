local runner = require("ravel.tests.runner").make_runner()
local TEST, run_all = runner.TEST, runner.run_all

local function same_word(a, b)
    if #a ~= #b then return false end
    for i = 1, #a do
        if a[i] ~= b[i] then return false end
    end
    return true
end

local M = {}

function M.main(ravel)
    TEST("class_ii_return_phase_tower", function()
        local sigma11 = {{0, 1, 2}, {0, 2}, {0}}
        local tower = ravel.return_phase.build(sigma11, 0, 4096)
        assert(#tower.return_words == 3)
        assert(same_word(tower.return_words[1], {0, 1, 2}))
        assert(same_word(tower.return_words[2], {0, 2}))
        assert(same_word(tower.return_words[3], {0}))
        assert(#tower.states == 6)
        assert(#tower.charpoly_low_first == 7)
    end)
    assert(run_all())
end

return M
