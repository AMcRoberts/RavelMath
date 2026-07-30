-- scripts/build_spectre_transition_table.lua
--
-- Build the per-Spectre transition table (9 x 14 x 2) from the
-- specmap data. For each parent hex kind L:
--   for each child index (0 = dominant Spectre, 1 = inner child if L = G):
--     for each Spectre edge N in 0..13:
--       record the destination hex kind and the parent hex edge that
--       the Spectre edge sits on.
--
-- The table is needed to make the BFS walk across adjacent Spectres
-- (not just adjacent hexes), which is the next step after phase 1
-- per docs/THEOREM_STATUS.md.
--
-- For the parent hexes that have a preserved SVG (G and S), we
-- extract the dominant 14-gon (and the G1 inner 14-gon for G) from
-- the path d-attribute, then classify each Spectre edge by the angle
-- of its midpoint from the centroid. The parent hex edge sectors
-- are at 60-degree intervals starting from the orientation implied by
-- the unit-hex vertex positions (see lua_src/ravel/data/hex_vertices.lua):
--   edge 0 spans 240..300 deg, edge 1 spans 300..360 deg, etc.
-- Each Spectre edge's sector then gives the neighbour hex kind via
-- the hex outer table.
--
-- For the other hexes (D, F, J, L, X, P, Y) the original specmap
-- SVGs are not preserved in the reference folder, so we fall back to
-- a label-position-based matching (closest edge.segment label) and
-- flag the result as provisional.

local M = {}

local function read_enriched(path)
    local labels = {}
    local f = io.open(path, "r")
    if not f then return labels end
    for line in f:lines() do
        if line:sub(1, 1) ~= "#" then
            local x, y, font, cls, content =
                line:match("^%(%s*([%-%d%.]+),%s*([%-%d%.]+)%)%s+font=%s*([%-%d%.]+)%s+inside=%s*(%S+)%s+label='(.+)'")
            if x then
                labels[#labels + 1] = {
                    x = tonumber(x), y = tonumber(y),
                    font = tonumber(font), inside = cls,
                    content = content,
                }
            end
        end
    end
    f:close()
    return labels
end

local function classify(label)
    if label.font >= 17.0 and label.font <= 19.0 then
        return "child_index", label.content
    elseif label.font >= 11.0 and label.font <= 13.0 then
        local n = tonumber(label.content)
        if n then return "spectre_edge", n end
    elseif label.font >= 8.0 and label.font <= 10.0 then
        local a, b = label.content:match("^(%d+)%.(%d+)$")
        if a then return "edge_segment", { tonumber(a), tonumber(b) } end
    end
    return nil
end

-- Parse a path d-attribute into a list of vertices.
local function parse_path_d(d)
    local verts = {}
    for x, y in d:gmatch("([%-%d%.]+)[%s,]+([%-%d%.]+)") do
        verts[#verts + 1] = { tonumber(x), tonumber(y) }
    end
    return verts
end

-- Extract the polygons with class "G", "G1", "D", "S", ... from an
-- SVG.  Returns a table { [class] = { verts = {...}, cx, cy } }.
local function parse_svg_polys(svg_path, wanted_classes)
    local f = io.open(svg_path, "r")
    if not f then return {} end
    local out = {}
    for line in f:lines() do
        local cls = line:match('class="([^"]+)"')
        local d   = line:match('d="([^"]+)"')
        if cls and d then
            local verts = parse_path_d(d)
            if #verts >= 3 and (not wanted_classes or wanted_classes[cls]) then
                local cx, cy = 0.0, 0.0
                for _, v in ipairs(verts) do cx, cy = cx + v[1], cy + v[2] end
                out[cls] = {
                    verts = verts,
                    cx = cx / #verts,
                    cy = cy / #verts,
                }
            end
        end
    end
    f:close()
    return out
end

-- The unit hex vertex angles from the centroid, in degrees, math
-- convention (positive x to positive y to negative x to negative y,
-- CCW). Vertices 0..5 are at multiples of 60 deg with vertex 0 at
-- (-0.5, -0.866) -> math angle -120 (240).
-- Hex edge N spans from vertex N to vertex N+1; the edge midpoint
-- direction is at (N + 0.5) * 60 - 90 deg, or equivalently the edge
-- is the sector whose angular boundaries are the two vertex angles.
local HEX_VERTEX_ANGLES = { 240, 300, 0, 60, 120, 180 }
local function sector_for_angle(theta_deg)
    local t = theta_deg % 360
    if t < 0 then t = t + 360 end
    for i = 0, 5 do
        local a = HEX_VERTEX_ANGLES[i + 1]
        local b = HEX_VERTEX_ANGLES[((i + 1) % 6) + 1]
        local lo, hi
        local tt = t
        if a < b then
            lo, hi = a, b
        else
            -- wraps around 360
            lo, hi = a, b + 360
            if tt < a then tt = tt + 360 end
        end
        if tt >= lo and tt < hi then
            return i
        end
    end
    return nil
end

local function atan2_deg(dy_math, dx)
    return math.deg(math.atan(dy_math, dx))
end

-- For each edge of the given polygon (cyclically), compute its
-- midpoint and the angle from the polygon's centroid.
local function polygon_edge_angles(poly)
    local n = #poly.verts
    local edges = {}
    for i = 0, n - 1 do
        local a = poly.verts[i + 1]
        local b = poly.verts[((i + 1) % n) + 1]
        local mx = (a[1] + b[1]) / 2
        local my = (a[2] + b[2]) / 2
        local dx = mx - poly.cx
        local dy_math = -(my - poly.cy)  -- screen y is inverted
        local theta = atan2_deg(dy_math, dx)
        edges[#edges + 1] = { mx = mx, my = my, theta = theta, sector = sector_for_angle(theta) }
    end
    return edges
end

-- For each 12px label on the dominant (or inner) polygon, find the
-- closest 14-gon edge by Euclidean distance, then look up that
-- edge's parent hex sector.
local function map_spectre_edges(poly, spectre_labels, debug)
    local edges = polygon_edge_angles(poly)
    if debug then
        io.stderr:write(string.format("polygon centroid: (%.2f, %.2f)\n", poly.cx, poly.cy))
        for i, e in ipairs(edges) do
            io.stderr:write(string.format(
                "  edge %2d mid=(%7.2f, %7.2f) theta=%7.2f sector=%s\n",
                i - 1, e.mx, e.my, e.theta, tostring(e.sector)))
        end
    end
    local out = {}
    for _, sl in ipairs(spectre_labels) do
        local best_i, best_dist = nil, math.huge
        for i, e in ipairs(edges) do
            local dx = sl.x - e.mx
            local dy = sl.y - e.my
            local d = dx * dx + dy * dy
            if d < best_dist then best_i, best_dist = i, d end
        end
        local e = edges[best_i]
        out[#out + 1] = {
            spectre_edge = sl.spectre_edge,
            edge_index = best_i - 1,  -- 0-based edge index in the 14-gon
            sector = e.sector,
            midx = e.mx, midy = e.my, theta = e.theta,
        }
        if debug then
            io.stderr:write(string.format(
                "  label edge=%2d at (%.2f, %.2f) -> 14-gon edge %d (sector=%s)\n",
                sl.spectre_edge, sl.x, sl.y,
                best_i and (best_i - 1) or -1,
                tostring(e and e.sector)))
        end
    end
    return out
end

-- Build the per-Spectre transition table for one parent hex kind.
-- Returns a list of records { child_index, spectre_edge, parent_edge,
--                              neighbour, source } sorted by
-- (child_index, spectre_edge).
local function build_table(labels, outer, kind, kind_ix)
    -- Collect dominant and inner Spectre child labels separately.
    local dominant_spectre = {}
    local inner_spectre    = {}
    local edge_segments    = {}
    -- Determine whether the labels come from an SVG-parsed enriched
    -- file (inside_class may equal the parent kind or "K1") or from a
    -- txt fallback (inside_class is the literal "nil").
    local has_svg = false
    for _, label in ipairs(labels) do
        if label.inside ~= "nil" then has_svg = true; break end
    end
    for _, label in ipairs(labels) do
        local t, v = classify(label)
        if t == "spectre_edge" then
            local ci = -1
            if label.inside == kind then
                ci = 0
            elseif label.inside == kind .. "1" then
                ci = 1
            elseif not has_svg then
                -- Fallback: no SVG, treat all ravel-edge labels as
                -- belonging to the dominant Spectre. Inner-child edges
                -- will be wrong but only G has an inner child, and G
                -- does have an SVG.
                ci = 0
            end
            if ci == 0 then
                dominant_spectre[#dominant_spectre + 1] = {
                    x = label.x, y = label.y, spectre_edge = v,
                }
            elseif ci == 1 then
                inner_spectre[#inner_spectre + 1] = {
                    x = label.x, y = label.y, spectre_edge = v,
                }
            end
        elseif t == "edge_segment" then
            edge_segments[#edge_segments + 1] = {
                x = label.x, y = label.y,
                parent_edge = v[1], segment = v[2],
            }
        end
    end

    -- Try the geometric (polygon-centroid) approach first; fall back
    -- to closest-9px for hexes without a preserved SVG.
    local dominant_recorded = {}
    local inner_recorded    = {}
    local project_dir = (arg and arg[0] and arg[0]:match("(.*/)") or "./")
    if project_dir == "" then project_dir = "./" end
    project_dir = project_dir .. ".."
    local svg_path = string.format("%s/../tatham_svg_parsed/specmap_%s.svg",
        project_dir, kind)
    local polys = parse_svg_polys(svg_path, { [kind] = true, [kind .. "1"] = true })

    if polys[kind] then
        dominant_recorded = map_spectre_edges(polys[kind], dominant_spectre,
            kind == "G")
    end
    if polys[kind .. "1"] then
        inner_recorded = map_spectre_edges(polys[kind .. "1"], inner_spectre,
            kind == "G")
    end

    local function nearest_edge_for(sl)
        local best, best_dist = nil, math.huge
        for _, es in ipairs(edge_segments) do
            local dx = sl.x - es.x
            local dy = sl.y - es.y
            local d = dx * dx + dy * dy
            if d < best_dist then best, best_dist = es, d end
        end
        return best
    end

    local records = {}
    local function add_records(spectre_labels, ci, recorded)
        if #recorded == #spectre_labels then
            for _, r in ipairs(recorded) do
                local rec = {
                    child_index = ci,
                    spectre_edge = r.spectre_edge,
                    parent_edge = r.sector,
                    source = "geometric",
                }
                if r.sector then
                    local outer_row = outer.lookup(kind_ix, r.sector, 0)
                    if outer_row then rec.neighbour = outer_row.neighbour end
                end
                records[#records + 1] = rec
            end
        else
            -- Fall back: closest edge.segment label.
            for _, sl in ipairs(spectre_labels) do
                local es = nearest_edge_for(sl)
                local rec = {
                    child_index = ci,
                    spectre_edge = sl.spectre_edge,
                    parent_edge = es and es.parent_edge or nil,
                    source = "closest-segment",
                }
                if es and es.parent_edge then
                    local outer_row = outer.lookup(kind_ix, es.parent_edge, 0)
                    if outer_row then rec.neighbour = outer_row.neighbour end
                end
                records[#records + 1] = rec
            end
        end
    end
    add_records(dominant_spectre, 0, dominant_recorded)
    add_records(inner_spectre,    1, inner_recorded)

    table.sort(records, function(a, b)
        if a.child_index ~= b.child_index then
            return a.child_index < b.child_index
        end
        return a.spectre_edge < b.spectre_edge
    end)
    return records
end

local KIND_INDICES = { G = 0, D = 1, J = 2, L = 3, X = 4,
                       P = 5, S = 6, F = 7, Y = 8 }

local function main(arg)
    local src_dir = arg[0]:match("(.*/)") or "./"
    if src_dir == "" then src_dir = "./" end
    local project_dir = src_dir .. ".."
    package.path = project_dir .. "/lua_src/?.lua;"
                  .. project_dir .. "/lua_src/?/init.lua;"
                  .. package.path
    local ravel = require("ravel").init({})
    local outer = ravel.outer
    local out_root = project_dir .. "/lua_src/ravel/data/spectre_transitions"
    os.execute("mkdir -p " .. out_root)

    local letters = { "G", "D", "J", "L", "X", "P", "S", "F", "Y" }
    local summary = {}
    for _, L in ipairs(letters) do
        local enriched_path = string.format(
            "%s/lua_src/ravel/data/specmap_enriched/specmap_%s.txt",
            project_dir, L)
        local labels = read_enriched(enriched_path)
        local kind_ix = KIND_INDICES[L]
        local records = build_table(labels, outer, L, kind_ix)
        summary[L] = records

        local out = io.open(string.format("%s/spectre_transitions_%s.txt",
            out_root, L), "w")
        out:write(string.format("# Per-Spectre transition table for parent hex %s\n", L))
        out:write("# columns: child_index spectre_edge parent_edge neighbour source\n")
        out:write("# child_index: 0 = dominant Spectre, 1 = inner child (G only)\n")
        out:write("# spectre_edge: 0..13 (Tatham's labelling)\n")
        out:write("# parent_edge: the parent hex edge (0..5) that this Spectre edge sits on\n")
        out:write("# neighbour: hex kind on the other side (from outer table)\n")
        out:write("# source: geometric (from polygon centroid) or closest-segment fallback\n")
        for _, r in ipairs(records) do
            out:write(string.format("child=%d edge=%2d parent_edge=%d neighbour=%-3s source=%s\n",
                r.child_index, r.spectre_edge,
                r.parent_edge or -1,
                r.neighbour and ravel.hex_kind.name(r.neighbour) or "?",
                r.source))
        end
        out:close()
    end
    io.write(string.format("done; per-Spectre transition tables written under %s\n",
        out_root))
    io.write("summary:\n")
    for _, L in ipairs(letters) do
        io.write(string.format("  %s: %d records\n", L, #(summary[L] or {})))
    end
end

if arg and arg[0] and arg[0]:match("build_spectre_transition_table%.lua$") then
    main(arg)
end

return {
    read_enriched = read_enriched,
    classify = classify,
    polygon_edge_angles = polygon_edge_angles,
    map_spectre_edges = map_spectre_edges,
    build_table = build_table,
}
