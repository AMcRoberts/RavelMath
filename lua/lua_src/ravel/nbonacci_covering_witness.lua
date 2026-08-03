-- lua/lua_src/ravel/nbonacci_covering_witness.lua
--
-- Thin Lua wrapper over the C++ covering-witness binding
-- (ravel.nbonacci.covering_witness.*).  The C++ side does the
-- actual symbolic work (Bareiss-like Laplace expansion over
-- SignInt = (num, denom) reduced to coprime form); the Lua side
-- orchestrates the calls for n = 2..8 and returns the simplest
-- covering witness at each n, used by the C++ proof probe to
-- emit Lean chunks via ravel.nbonacci.covering_witness.get_batch.
--
-- Convention: the function returns nil at n where the C++ side
-- (the bareiss/laplace search at the requested L) finds no
-- covering witness, in which case the Lean side skips that n.

local M = {}
local cw = ravel.nbonacci_covering_witness

-- The simplest covering witness at a single (n, L).  The C++ side
-- returns nil if none exists at this (n, L).
function M.get_simplest(n, L)
    L = L or (n + 1)
    return cw.get_simplest(n, L)
end

-- A batch of simplest covering witnesses at n = n_min..n_max with
-- L = n+1.  Returns a list of n_max - n_min + 1 records (each either
-- a table {n, found, indices, signs, free_params, sequence} or
-- nil if the C++ side found no witness at that n).
function M.get_batch(n_min, n_max)
    return cw.get_batch(n_min, n_max)
end

-- Box and cover check for a candidate record.  Returns true if
-- the candidate satisfies both properties (the C++ side oracle).
function M.check(indices, signs, free_params, sequence, n)
    return cw.check(indices, signs, free_params, sequence, n)
end

-- The discrete-chunk generator pattern.  This is the "Lua file"
-- part of the C++/Lua division of labor: the C++ side computes
-- the symbolic data (the witness fields, the box/cover verification),
-- and the Lua side distributes the data to chunks for the Lean
-- side.  Each iteration emits a chunk of Lua values (a table)
-- that the C++ proof probe consumes.
--
-- Returns a list of "chunks", one per n in [n_min, n_max], with
-- each chunk being a table {n, L, found, indices, signs, free_params,
-- sequence, box_ok, cover_ok, ready_for_lean=Bool}.  The
-- ready_for_lean flag is set if box and cover both pass (i.e.
-- the candidate is a valid covering witness).
function M.chunks(n_min, n_max)
    local witnesses = cw.get_batch(n_min, n_max)
    local chunks = {}
    for idx, w in ipairs(witnesses) do
        local n = n_min + idx - 1
        local L = n + 1
        local chunk = {
            n = n, L = L, found = (w ~= nil),
            indices = w and w.indices or {},
            signs = w and w.signs or {},
            free_params = w and w.free_params or {},
            sequence = w and w.sequence or {}
        }
        if w then
            chunk.box_ok = (cw.check(
                w.indices, w.signs, w.free_params, w.sequence, n) == 1)
            chunk.cover_ok = chunk.box_ok
            chunk.ready_for_lean = chunk.cover_ok
        else
            chunk.box_ok = false
            chunk.cover_ok = false
            chunk.ready_for_lean = false
        end
        table.insert(chunks, chunk)
    end
    return chunks
end

return M
