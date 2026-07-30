-- lua/scripts/parse_specmap.lua
--
-- Parse a Tatham specmap_<letter>.svg and emit an enriched label list
-- that associates every text label with (1) the path it is contained
-- inside and (2) the path's CSS class (which is the destination hex
-- kind: G, G1, F, Y, S, D, P, X, J, L).
--
-- The output is written as an enriched companion file under
-- lua_src/ravel/data/specmap_enriched/<letter>.txt. Each line is one
-- label with fields:
--   (x, y) font_size class path_centroid
-- and the trailing original label content.
--
-- The hex outer transition table alone cannot determine destinations
-- for the 12 px Spectre-edge labels (they sit at edges of the
-- dominant 14-gon, not inside the parent hex's hex boundary). To
-- recover the destination we still need the colour information from
-- the SVG, so the enriched companion is the input to the next step of
-- phase 2: building the per-Spectre transition table.
--
-- This script is run from the lua_reimplementation directory as
-- `lua5.4 scripts/parse_specmap.lua` (no arguments needed).

local M = {}

-- Extract the class attribute from a path element.
local function path_class(line)
    return line:match('class="([^"]+)"')
end

-- Extract the polygon vertices from a path's d-attribute. Supports
-- the absolute `M x y L x y ...` form used by Tatham.
local function parse_path_d(d)
    local verts = {}
    -- Tokenize: 'M' 'L' 'z' and signed decimals.
    for x, y in d:gmatch("([%-%d%.]+)[%s,]+([%-%d%.]+)") do
        verts[#verts + 1] = { tonumber(x), tonumber(y) }
    end
    return verts
end

-- Even-odd rule point-in-polygon test.  Returns true if (px, py) is
-- strictly inside the polygon; boundary counts as inside.
local function point_in_polygon(px, py, verts)
    local n = #verts
    if n < 3 then return false end
    local inside = false
    local j = n
    for i = 1, n do
        local xi, yi = verts[i][1], verts[i][2]
        local xj, yj = verts[j][1], verts[j][2]
        if ((yi > py) ~= (yj > py))
        and (px < (xj - xi) * (py - yi) / (yj - yi + 1e-30) + xi) then
            inside = not inside
        end
        j = i
    end
    return inside
end

local function polygon_centroid(verts)
    local cx, cy, n = 0.0, 0.0, 0
    for _, v in ipairs(verts) do
        cx = cx + v[1]; cy = cy + v[2]; n = n + 1
    end
    return cx / n, cy / n
end

local function parse_svg(svg_path)
    local paths = {}     -- list of {class=..., verts=...}
    local texts = {}     -- list of {x, y, font_size, content}
    local file = io.open(svg_path, "r")
    if not file then
        error("parse_specmap: cannot open " .. svg_path)
    end
    for line in file:lines() do
        -- Path element
        local cls = path_class(line)
        local d = line:match('d="([^"]+)"')
        if cls and d then
            local verts = parse_path_d(d)
            paths[#paths + 1] = {
                class = cls,
                verts = verts,
                cx, cy = polygon_centroid(verts),
            }
            local last = paths[#paths]
            last.cx, last.cy = polygon_centroid(last.verts)
        end
        -- Text element
        local x, y = line:match('x="([%-%d%.]+)"[^/]-y="([%-%d%.]+)"')
        local font = line:match('font%-size:%s*([%-%d%.]+)px')
        local content = line:match('>([^<]+)</text>')
        if x and y and font and content then
            texts[#texts + 1] = {
                x = tonumber(x),
                y = tonumber(y),
                font_size = tonumber(font),
                content = content,
            }
        end
    end
    file:close()
    return paths, texts
end

-- For each text label, find which path it is inside. If a label is
-- not inside any path (rare), we report nil so the caller can choose
-- how to handle it.
local function enrich_labels(paths, texts)
    local out = {}
    for _, t in ipairs(texts) do
        local inside_class = nil
        for _, p in ipairs(paths) do
            if point_in_polygon(t.x, t.y, p.verts) then
                inside_class = p.class
                break
            end
        end
        out[#out + 1] = {
            x = t.x,
            y = t.y,
            font_size = t.font_size,
            content = t.content,
            inside_class = inside_class,
        }
    end
    return out
end

-- CLI entry point.
local function main(arg)
    local src_dir = arg[0]:match("(.*/)") or "./"
    if src_dir == "" then src_dir = "./" end
    local project_dir = src_dir .. ".."
    local svg_root = project_dir .. "/../tatham_svg_parsed"
    local txt_root = project_dir .. "/../tatham_svg_parsed"
    local out_root = project_dir .. "/lua_src/ravel/data/specmap_enriched"
    -- Make sure output directory exists.
    os.execute("mkdir -p " .. out_root)
    local letters = { "G", "D", "J", "L", "X", "P", "S", "F", "Y" }
    local written = 0
    for _, L in ipairs(letters) do
        local svg = string.format("%s/specmap_%s.svg", svg_root, L)
        local f = io.open(svg, "r")
        if f then
            f:close()
            local paths, texts = parse_svg(svg)
            local enriched = enrich_labels(paths, texts)
            local out = io.open(string.format("%s/specmap_%s.txt", out_root, L), "w")
            out:write(string.format("# specmap_%s enriched labels (from SVG)\n", L))
            out:write(string.format("# font_size -> meaning:\n"))
            out:write(string.format("#   18 px : dominant Spectre child index (drawn on the parent path)\n"))
            out:write(string.format("#   12 px : Spectre edge number 0..13\n"))
            out:write(string.format("#    9 px : parent hex edge.segment pair (a.b)\n"))
            out:write(string.format("# inside_class : the CSS class of the path the label sits in\n"))
            for _, e in ipairs(enriched) do
                out:write(string.format(
                    "(%7.2f, %7.2f) font=%5.1f inside=%-3s label='%s'\n",
                    e.x, e.y, e.font_size, e.inside_class or "nil", e.content))
            end
            out:close()
            io.write(string.format("wrote %s/specmap_%s.txt (%d labels, %d paths)\n",
                out_root, L, #enriched, #paths))
            written = written + 1
        else
            -- Fall back: derive enriched from the pre-parsed txt file.
            -- The txt file lists "(x, y): 'label'" but doesn't include
            -- the font size.  We use a heuristic based on the label
            -- content: single digits 0..13 are likely 12 px (Spectre
            -- edge numbers) if they appear in a specmap; '0.0'-style
            -- are 9 px edge.segment labels; everything else is 18 px.
            local txt = string.format("%s/specmap_%s.txt", txt_root, L)
            local f2 = io.open(txt, "r")
            if not f2 then
                io.write(string.format("(skip %s: no SVG or txt)\n", L))
            else
                local out = io.open(string.format("%s/specmap_%s.txt", out_root, L), "w")
                out:write(string.format("# specmap_%s enriched labels (from txt, no SVG)\n", L))
                out:write("# font_size inferred from label content (no SVG = no inside_class)\n")
                out:write("# The single-digit label nearest the 14-gon centroid is\n")
                out:write("# the 18 px child-index label and is dropped from the output.\n")
                -- Find the centroid: for most hexes the 18 px label is at
                -- G's centroid (76.27, 122.08); S is at (113.77, 122.08).
                local centroid_x = (L == "S") and 113.77 or 76.27
                local centroid_y = 122.08
                -- First pass: find the single-digit label nearest to the
                -- centroid.  This is the 18 px label.
                local best_dist = math.huge
                local best_content
                for line in f2:lines() do
                    local x, y, content =
                        line:match("^%(([%-%d%.]+),%s*([%-%d%.]+)%)%:%s*'(.+)'")
                    if x and content:match("^%d+$") then
                        local dx = tonumber(x) - centroid_x
                        local dy = tonumber(y) - centroid_y
                        local d = dx * dx + dy * dy
                        if d < best_dist then
                            best_dist = d
                            best_content = content
                        end
                    end
                end
                f2:close()
                local f3 = io.open(txt, "r")
                local count = 0
                for line in f3:lines() do
                    local x, y, content =
                        line:match("^%(([%-%d%.]+),%s*([%-%d%.]+)%)%:%s*'(.+)'")
                    if x then
                        -- Skip the 18 px child-index label.
                        if content == best_content
                        and math.abs(tonumber(x) - centroid_x) < 1.0
                        and math.abs(tonumber(y) - centroid_y) < 1.0 then
                            -- drop
                        else
                            local font
                            if content:match("^%d+%.%d+$") then
                                font = 9
                            elseif tonumber(content) and tonumber(content) <= 13 then
                                font = 12
                            else
                                font = 18
                            end
                            out:write(string.format(
                                "(%7.2f, %7.2f) font=%5.1f inside=%-3s label='%s'\n",
                                tonumber(x), tonumber(y), font, "nil", content))
                            count = count + 1
                        end
                    end
                end
                f3:close()
                out:close()
                io.write(string.format("wrote %s/specmap_%s.txt (%d labels, from txt fallback)\n",
                    out_root, L, count))
                written = written + 1
            end
        end
    end
    io.write(string.format("done; %d specmap enriched file(s) written.\n", written))
end

if arg and arg[0] and arg[0]:match("parse_specmap%.lua$") then
    main(arg)
end

return {
    parse_svg = parse_svg,
    parse_path_d = parse_path_d,
    point_in_polygon = point_in_polygon,
    enrich_labels = enrich_labels,
}
