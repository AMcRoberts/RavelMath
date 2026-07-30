-- lua/tests/test_hex.lua
--
-- HexKind enum and child counts. Mirrors tests/test_spectre_hex.cpp.

local runner = require("ravel.tests.runner")
local R = runner.make_runner()
local TEST = R.TEST

local EQ = runner.assert_eq
local T = runner.assert_true

local hex

local function spectre_hex_kind_naming()
    EQ(hex.name(hex.G),     "G")
    EQ(hex.name(hex.D),     "D")
    EQ(hex.name(hex.J),     "J")
    EQ(hex.name(hex.L),     "L")
    EQ(hex.name(hex.X),     "X")
    EQ(hex.name(hex.P),     "P")
    EQ(hex.name(hex.S),     "S")
    EQ(hex.name(hex.F),     "F")
    EQ(hex.name(hex.Y),     "Y")
    EQ(hex.name(hex.COUNT), "?")

    local kinds = hex.all_kinds()
    EQ(#kinds, 9)
    EQ(kinds[1], hex.G)
    EQ(kinds[9], hex.Y)
end

local function spectre_hex_child_count()
    EQ(hex.child_count(hex.G), 2)
    for _, k in ipairs({ hex.D, hex.J, hex.L, hex.X,
                         hex.P, hex.S, hex.F, hex.Y }) do
        EQ(hex.child_count(k), 1)
    end
end

local function main(spectre_module)
    hex = spectre_module.hex_kind
    TEST("spectre_hex_kind_naming",  spectre_hex_kind_naming)
    TEST("spectre_hex_child_count",  spectre_hex_child_count)
    return R.run_all()
end

return { main = main }
