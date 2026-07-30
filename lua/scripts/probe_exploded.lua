-- scripts/probe_exploded.lua
-- Test the 12 previously-EXPLODED entries under the in_H_sigma fix
-- (tighter upper-bound tolerance: 1e-10 instead of 0).  See
-- docs/RESEARCH_STATUS.md.

local script_dir = arg and arg[0] and arg[0]:match("(.*/)") or "./"
if script_dir == "" then script_dir = "./" end
local project_dir = script_dir .. ".."
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;"
              .. project_dir .. "/lua_src/?/init.lua;"
              .. package.path

local native = require("spectre_native")
local ravel = require("ravel").init(native)

-- The 12 EXPLODED entries from FINDINGS_contact_boundary_survey.md
-- (all |det|=2, |b₂| in [0.71, 0.92]).  Subs read directly from the
-- JSONL source.
local EXPLODED = {
    {"rnd1_canon",  {{1}, {0,2,2}, {0,0,1,2,2}},                3.152757602010394, 0.7964705223757771},
    {"rnd4_canon",  {{0,0,1,2}, {2}, {0,0}},                    2.919639565839417,  0.8276569716592435},
    {"rnd5_canon",  {{0,1,1,2}, {0,0,1,1}, {0,1,1}},          3.90057187491196,   0.7160623762063814},
    {"rnd6_canon",  {{0,1}, {0,0,1,1,2,2}, {0}},                3.195823345445647,  0.791085848033237},
    {"rnd8_canon",  {{0,1,2}, {0,1,1}, {1,2}},                  2.839286755214159,  0.8392867552141606},
    {"rnd8_barge",  {{0,2,1}, {1,0,1}, {2,1}},                  2.839286755214159,  0.8392867552141606},
    {"rnd10_canon", {{1,2}, {2,2}, {0,1,2}},                    2.5115471416945288, 0.8923687036530592},
    {"rnd13_canon", {{0,0,1,1,2}, {0,0,1,1,2,2}, {1}},        4.353855785430829,  0.8774037457000372},
    {"rnd16_canon", {{0,0,2}, {0,0,1}, {1}},                    2.5213797068045687, 0.8906270293855812},
    {"rnd19_canon", {{0,2}, {0,0}, {0,1,2}},                    2.359304085971777,  0.9207103769044674},
    {"rnd19_barge", {{2,0}, {0,0}, {1,2,0}},                    2.359304085971777,  0.9207103769044674},
    {"rnd24_canon", {{0,1,1,1}, {0,0,1,0}, {1,0,1,1}},        2.521379706804568,  0.8906270293855811},
}

for idx, e in ipairs(EXPLODED) do
    local name, subst, beta, b2 = e[1], e[2], e[3], e[4]
    local t0 = os.clock()
    local ok, rep = pcall(function()
        return ravel.contact_boundary.from_subst(subst, 2, 2000, 5000, beta, b2)
    end)
    local dt = os.clock() - t0
    if not ok then
        print(string.format("[%2d] %-15s  ERROR: %s  (dt=%.2fs)", idx, name, tostring(rep), dt))
    else
        print(string.format(
            "[%2d] %-15s  dt=%6.2fs  |D|=%-3d |C|=%-5d |±C|=%-5d |G_B|=%-5d  BP=%g  λ=%g  %s",
            idx, name, dt, rep.d_cont_size, rep.contact_size,
            rep.signed_contact_size, rep.boundary_size,
            rep.bp_rho_nc, rep.boundary_eigenvalue,
            rep.converged and "OK" or "uncov"))
    end
end
