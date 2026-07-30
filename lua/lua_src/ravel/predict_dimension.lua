-- lua/ravel/predict_dimension.lua
--
-- Conformal Pisot boundary-dimension prediction.
-- Mirrors python/boundary_dimension_shortcut.py::predict_boundary_dimension.
--
-- For an irreducible unimodular Pisot substitution with a *single*
-- secondary modulus (the "conformal" case -- a complex conjugate pair,
-- or two equal real conjugates), the conjectural identity
--
--     rho_nc (balanced-pair) == boundary automaton Perron eigenvalue
--     s_H = 2 - log(beta / rho_nc) / log(1 / |alpha|)
--
-- has been verified on the sigma_{a,b} cubic family and Tetrabonacci
-- quartic (see ANSWERS §3.5). For the November-2025 ambient graph
-- this conjecture is open (see ANSWERS §3.1); we tag conformal
-- predictions accordingly.
--
-- Non-conformal regime (secondary spectrum has more than one distinct
-- modulus): the dimension formula doesn't apply, but rho_nc is still
-- informative for the conjecture certificate (certified -> rho_nc < beta).

local M = {}

-- Compute the conformal predicted Rauzy fractal boundary dimension.
--   subst:   { 1: {word_int_list}, 2: {...}, ... }
--   rho_nc:  the balanced-pair Perron eigenvalue (already computed)
--   returns: { conformal=bool, beta=..., b2=..., |alpha|=...,
--             s_H=...? , rho_nc=..., status="..." }
--
-- status is one of:
--   "PREDICTION (conformal, single-modulus formula)"
--   "NON-CONFORMAL secondary spectrum -- rho_nc computed, no
--    dimension formula applies"
--   "NOT PISOT (|b2| >= 1) -- no prediction; not Pisot substitution"
function M.predict(subst, rho_nc)
    local n = #subst
    local sp = require("ravel")
    -- Build the substitution incidence matrix M[r+1][c+1] (1-indexed Lua
    -- convention) = count of letter r in subst[c]'s image.  Rows are
    -- pre-filled with zeros so the C++ spectral_invariants_3x3 helper
    -- sees a fully-populated 3x3 table (it indexes rawgeti(1..3, 1..3)).
    local mat = {}
    for r = 1, n do
        mat[r] = {}
        for c = 1, n do mat[r][c] = 0 end
    end
    for c = 1, n do
        for _, l in ipairs(subst[c]) do
            mat[l + 1][c] = mat[l + 1][c] + 1
        end
    end

    local spectral
    if n == 2 then
        -- 2-letter Pisot: rows of mat are letters 0 and 1, columns
        -- are images of letters 0 and 1.  spectral.invariants_2x2 expects
        -- (M[0][0], M[0][1], M[1][0], M[1][1]) (C-side 0-indexed,
        -- we built mat 1-indexed so shift by 1).
        spectral = sp.spectral.invariants_2x2(mat[1][1] or 0, mat[1][2] or 0,
                                              mat[2][1] or 0, mat[2][2] or 0)
    elseif n == 3 then
        spectral = sp.spectral.invariants_3x3(mat)
    else
        return { conformal = false, status = "n > 3 not supported by predict_dimension" }
    end

    local beta     = spectral.beta_abs
    local b2       = spectral.beta2
    local abs_b2   = math.abs(b2)

    if not (beta > 1.0) then
        return { conformal = false, beta = beta, b2 = abs_b2,
                 rho_nc = rho_nc, status = "NOT PISOT (beta <= 1) -- no prediction" }
    end

    if not (abs_b2 < 1.0 - 1e-9) then
        return { conformal = false, beta = beta, b2 = abs_b2,
                 rho_nc = rho_nc,
                 status = "NOT PISOT (|b2| >= 1) -- no prediction; non-Pisot substitution" }
    end

    -- Determine conformal: secondary spectrum has ONE modulus class.
    -- For a 3-letter cubic Pisot, complex pair iff |b2| = beta^(-1/2).
    local conformal
    local alpha_mod
    if n == 2 then
        conformal = true
        alpha_mod = abs_b2
    else
        local expected_complex = math.pow(beta, -0.5)
        if math.abs(abs_b2 - expected_complex) / expected_complex < 1e-3 then
            conformal = true
            alpha_mod = abs_b2
        else
            conformal = false
        end
    end

    if conformal then
        if rho_nc and rho_nc > 0 and alpha_mod > 0 and alpha_mod < 1.0 then
            local s_H = 2.0 - math.log(beta / rho_nc) / math.log(1.0 / alpha_mod)
            return { conformal = true, beta = beta, b2 = abs_b2,
                     alpha = alpha_mod, rho_nc = rho_nc,
                     s_H = s_H,
                     status = "PREDICTION (conformal, single-modulus formula)" }
        else
            return { conformal = true, beta = beta, b2 = abs_b2,
                     status = "CONFORMAL but rho_nc or |alpha| invalid -- no prediction" }
        end
    else
        return { conformal = false, beta = beta, b2 = abs_b2,
                 rho_nc = rho_nc,
                 status = "NON-CONFORMAL secondary spectrum -- rho_nc computed, "
                          .. "dimension needs Falconer/multi-modulus machinery, not attempted" }
    end
end

-- A small fixed sweep, mirroring boundary_dimension_shortcut.py's __main__.
-- Returns a list of records {name, beta, b2, conformal, rho_nc, s_H, status}.
function M.sweep(max_pairs, max_len)
    max_pairs = max_pairs or 20000
    max_len   = max_len   or 60000
    local records = {}
    local menu = {
        fibonacci      = { {0, 1}, {0} },
        tribonacci     = { {0, 1}, {0, 2}, {0} },
        supergolden    = { {0, 1}, {2},   {0} },
        sigma_4_2      = { {0, 0, 0, 0, 1}, {0, 0, 2}, {0} },
        tetrabonacci    = { {0, 1}, {0, 2}, {0, 3}, {0} },
    }
    local sp = require("ravel")
    for name, subst in pairs(menu) do
        if name == "tetrabonacci" then
            -- 4-letter; spectral helpers only support 2x2 / 3x3.
            -- TODO(W11): extend spectral_invariants_4x4 once that lands.
            table.insert(records, {
                name = name,
                conformal = false,
                status = "Tetrabonacci (4-letter) -- not supported by current C++ helpers; " ..
                         "would need 4x4 spectral helper (deferred; see WORK_ITEMS W11)"})
        else
            local ok, err = pcall(function()
                local rho = sp.balanced_pair.rho_nc(subst, max_pairs, max_len)
                local cert = sp.balanced_pair.certify(subst, max_pairs, max_len)
                local result = M.predict(subst, rho)
                result.name = name
                result.certified = cert.certified
                result.n_irreducible = cert.n_irreducible
                table.insert(records, result)
            end)
            if not ok then
                table.insert(records, { name = name, status = "ERROR: " .. tostring(err) })
            end
        end
    end
    return records
end

return M
