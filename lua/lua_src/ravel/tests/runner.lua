-- lua/tests/runner.lua
--
-- Minimal hand-rolled test runner, modelled on
-- include/ravel/simple_test.hpp. Each test is a function that
-- either returns silently (pass) or throws via assert() (fail).
-- A test case is registered with TEST(name, fn) and the runner
-- calls run_all() to execute them in order.
--
-- Each call to make_runner() returns a fresh { TEST, run_all } pair
-- so that test files do not share a global registry. This avoids
-- test pollution between Lua test files when each file requires
-- the runner module.

local M = {}

function M.make_runner()
    local registry = {}

    local function TEST(name, fn)
        registry[#registry + 1] = { name = name, fn = fn }
    end

    local function run_all()
        local passed, failed = 0, 0
        for _, c in ipairs(registry) do
            io.write(string.format("[ RUN      ] %s\n", c.name))
            local ok, err = pcall(c.fn)
            if ok then
                io.write(string.format("[       OK ] %s\n", c.name))
                passed = passed + 1
            else
                io.write(string.format("[  FAILED  ] %s\n  %s\n", c.name, tostring(err)))
                failed = failed + 1
            end
        end
        io.write(string.format("\n[==========] %d passed, %d failed, %d total\n",
            passed, failed, passed + failed))
        return failed == 0
    end

    return { TEST = TEST, run_all = run_all }
end

-- Backwards-compatible single-runner interface (legacy).
M.registry = {}
M.TEST = function(name, fn) M.registry[#M.registry + 1] = { name = name, fn = fn } end
M.run_all = function()
    local passed, failed = 0, 0
    for _, c in ipairs(M.registry) do
        io.write(string.format("[ RUN      ] %s\n", c.name))
        local ok, err = pcall(c.fn)
        if ok then
            io.write(string.format("[       OK ] %s\n", c.name))
            passed = passed + 1
        else
            io.write(string.format("[  FAILED  ] %s\n  %s\n", c.name, tostring(err)))
            failed = failed + 1
        end
    end
    io.write(string.format("\n[==========] %d passed, %d failed, %d total\n",
        passed, failed, passed + failed))
    return failed == 0
end

-- Assertions. The double-underscore variants auto-print both sides
-- of an inequality for easier debugging.
local function fmt(x)
    if type(x) == "table" then
        -- try a __tostring metamethod, otherwise JSON-like
        local mt = getmetatable(x)
        if mt and mt.__tostring then return tostring(x) end
        local parts = {}
        for k, v in pairs(x) do parts[#parts + 1] = string.format("%s=%s", tostring(k), tostring(v)) end
        return "{" .. table.concat(parts, ", ") .. "}"
    end
    return tostring(x)
end

function M.assert_eq(actual, expected, msg)
    if actual ~= expected then
        error(string.format("assert_eq: %s\n  actual:   %s\n  expected: %s",
            msg or "", fmt(actual), fmt(expected)), 2)
    end
end

function M.assert_ne(actual, expected, msg)
    if actual == expected then
        error(string.format("assert_ne: %s\n  both are: %s",
            msg or "", fmt(actual)), 2)
    end
end

function M.assert_near(actual, expected, eps, msg)
    local d = actual - expected
    if not (d > -eps and d < eps) then
        error(string.format("assert_near: %s\n  actual:   %.17g\n  expected: %.17g\n  diff:     %.17g",
            msg or "", actual, expected, d), 2)
    end
end

function M.assert_true(cond, msg)
    if not cond then
        error("assert_true: " .. (msg or "condition is false"), 2)
    end
end

function M.assert_false(cond, msg)
    if cond then
        error("assert_false: " .. (msg or "condition is true"), 2)
    end
end

function M.assert_lt(a, b, msg)
    if not (a < b) then
        error(string.format("assert_lt: %s\n  %s is not < %s", msg or "", fmt(a), fmt(b)), 2)
    end
end

function M.assert_le(a, b, msg)
    if not (a <= b) then
        error(string.format("assert_le: %s\n  %s is not <= %s", msg or "", fmt(a), fmt(b)), 2)
    end
end

return M
