-- lua/data/constants.lua
--
-- Named constants for the Lua reimplementation. The decimal mantissa
-- of sqrt(3)/2 lives only in the C++ backend; the rest of the
-- project reads these names through ravel.constants.
--
-- Source of truth: src/cyclotomic.hpp (C++) and
-- docs/THEOREM_STATUS.md.

local M = {}

-- Forward declarations; populated by ravel.init().
M.HALF        = nil  -- 1/2
M.ONE         = nil  -- 1.0
M.ZERO        = nil  -- 0.0
M.SQRT3_OVER_2 = nil -- 0.86602540378443864676 (binary64 nearest)
M.D_X         = nil  -- cos(pi/6) = sqrt(3)/2
M.D_Y         = nil  -- sin(pi/6) = 1/2
M.D2_X        = nil  -- cos(pi/3) = 1/2
M.D2_Y        = nil  -- sin(pi/3) = sqrt(3)/2
M.D3_X        = nil  -- cos(pi/2) = 0
M.D3_Y        = nil  -- sin(pi/2) = 1

function M.load(native)
    M.HALF         = native.constants.half()
    M.ONE          = native.constants.one()
    M.ZERO         = native.constants.zero()
    M.SQRT3_OVER_2 = native.constants.sqrt3over2()
    M.D_X          = native.constants.d_x()
    M.D_Y          = native.constants.d_y()
    M.D2_X         = native.constants.d2_x()
    M.D2_Y         = native.constants.d2_y()
    M.D3_X         = native.constants.d3_x()
    M.D3_Y         = native.constants.d3_y()
end

return M
