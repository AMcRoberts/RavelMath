-- lua_src/ravel/init.lua
--
-- Top-level Lua namespace. Loads the C++ backend (libspectre_native.so)
-- and exposes a small Lua-friendly surface. Everything else in this
-- project goes through `ravel.cyclotomic.*`, `ravel.constants.*`,
-- `ravel.spectral.*`, `ravel.tilt.*`, `ravel.substitution.*`,
-- `ravel.balanced_pair.*`, `ravel.thermometer.*`, and
-- `ravel.barge.*`.
--
-- Conventions:
--   * Cyclotomic values are tables {a, b, c, e} of integers, returned
--     by all C++ cyclotomic.* calls. Lua-side arithmetic helpers
--     wrap the C++ calls so the user never has to spell out the indices.
--   * Constants are double-precision numbers pulled from the C++ side;
--     no decimal mantissa is typed in Lua.
--   * Spectral invariants are tables { n, beta, beta_abs, beta2,
--     det_M, abs_det, invol_safe, bound_rhs, bound_holds }.
--   * Substitutions are passed to the C++ as { 1: {word}, ... }
--     (1-indexed list of int words).
--   * Pisot theorem results are returned as { theta=..., records=... }
--     (records is a list of (lag, 2-smax) pairs in lag order).
--   * The hex outer transition table, the substitution rule, the
--     lineage helpers, and the BFS are pure Lua and live in
--     lua_src/ravel/*.

local M = {}

M.native              = nil
M.constants           = require("ravel.data.constants")
M.hex_kind            = require("ravel.data.hex_kind")
M.outer               = require("ravel.data.hex_outer_table")
M.vertices            = require("ravel.data.hex_vertices")
M.substitution_rule   = require("ravel.substitution_rule")
M.lineage             = require("ravel.lineage")
M.coord_bfs           = require("ravel.coord_bfs")
M.spectre_transitions = require("ravel.data.spectre_transitions")

-- cyclotomic helpers (Lua-side aliases for the C++ backend)
local native_cyclo = nil  -- populated after require

function M.init(native)
    M.native = native
    native_cyclo = native.cyclotomic
    if native.constants then
        M.constants.load(native)
        -- Aliases for cyclotomic helpers, wrapping the C++ table format
        -- into Lua arithmetic that feels natural.
        M.cyclo = {
            zero    = function() return native_cyclo.zero() end,
            one     = function() return native_cyclo.one() end,
            d       = function() return native_cyclo.d() end,
            d2      = function() return native_cyclo.d2() end,
            d3      = function() return native_cyclo.d3() end,
            make    = function(a, b, c, e) return native_cyclo.make(a, b, c, e) end,
            add     = function(x, y) return native_cyclo.add(x, y) end,
            sub     = function(x, y) return native_cyclo.sub(x, y) end,
            mul     = function(x, y) return native_cyclo.mul(x, y) end,
            neg     = function(x) return native_cyclo.neg(x) end,
            scale   = function(x, k) return native_cyclo.scale(x, k) end,
            eq      = function(x, y) return native_cyclo.eq(x, y) end,
            tostring = function(x) return native_cyclo.tostring(x) end,
            to_xy   = function(x) return native_cyclo.to_xy(x) end,
        }
M.spectral      = native.spectral
    M.tilt          = native.tilt
    M.substitution  = native.substitution
    M.return_phase  = native.return_phase
    M.balanced_pair = native.balanced_pair
    M.fibonacci_selection = native.fibonacci_selection
    M.fibonacci_finite = native.fibonacci_finite
    M.fibonacci_dynamics = native.fibonacci_dynamics
    M.thermometer   = native.thermometer
    M.barge         = native.barge
    M.survey        = native.survey
    M.contact_boundary = native.contact_boundary
    M.rauzy_fractal = native.rauzy_fractal
    M.d_cont_check  = native.d_cont_check
    M.constants._cyclo = M.cyclo
        -- predict_dimension is pure-Lua (W1 + W5)
        M.predict_dimension = require("ravel.predict_dimension")
    end
    return M
end

return M
