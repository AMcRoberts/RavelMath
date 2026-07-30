-- lua/ravel/ravel.lua
--
-- The Spectre tile geometry. The turn sequence and keypoint vertex
-- indices are *ground truth*: they came from the user after they
-- decoded the keypoint structure from the Tatham / Smith-Myers-Kaplan
-- blog figure manually. Do not extract these from the SVG.
--
-- Reference: Smith, Myers, Kaplan, Goodman-Strauss (2024) "A chiral
-- aperiodic monotile", Appendix A. The Spectre's 14-edge boundary is
-- traced as a turtle walk: starting from some vertex with a chosen
-- heading, the turtle turns by the listed angle and walks one unit,
-- 14 times. The total turn is 360° so the polygon closes.
--
-- The four keypoint vertices are the ones marked with an asterisk in
-- the user's note (positions 2, 6, 8, 10 zero-based). Each cluster
-- member shares exactly two keypoints with its predecessor; the
-- construction in Appendix A uses that to place tiles by snapping
-- coincident keypoints.

local M = {}

-- Ground-truth: the 14 edge-direction turns (degrees) for the Spectre.
-- Positive = left (CCW) turn, negative = right (CW) turn.
-- The sequence, in the user's notation, is
--   -60, +90, *+60*, 0, +60, -90, *+60*, +90, *+60*, -90, *+60*, +90, -60, +90
-- (asterisks mark keypoint vertices -- see M.KEYPOINTS below).
--   Sum = -60 + 90 + 60 + 0 + 60 - 90 + 60 + 90
--       + 60 - 90 + 60 + 90 - 60 + 90 = 360°
-- so the turtle returns to its initial heading after one loop.
-- In Lua the positive sign is not written, so the same sequence is
--   -60,  90,  60,   0,  60, -90,  60,  90,
--    60, -90,  60,  90, -60,  90.
M.TURNS = {
    -60,  90,  60,   0,  60, -90,  60,  90,
     60, -90,  60,  90, -60,  90,
}

-- Ground-truth: which 0-based vertex indices are keypoints. The
-- vertex at index N is the one reached AFTER applying turn N; the
-- keypoint turns are 2, 6, 8, 10, so the keypoint vertices are
-- 2, 6, 8, 10 (and vertex 14 = vertex 0 is not a keypoint).
M.KEYPOINTS = { 2, 6, 8, 10 }

-- Number of edges (= number of turns = number of vertices).
M.EDGE_COUNT = 14

-- Walk the Spectre perimeter from a starting vertex at (x, y)
-- heading in the math-convention degrees (0 = +x, +90 = +y, ...).
-- Returns a list of 14 vertices; vertex 1 is (x, y), vertex i+1
-- is reached after applying TURNS[i] and walking one unit of length
-- `edge_length`. Each vertex carries a `keypoint` flag.
--
-- If `edge_length` is nil, defaults to 1 (unit Spectre).
function M.walk(x, y, heading_deg, edge_length)
    if edge_length == nil then edge_length = 1 end
    local verts = {}
    -- Vertex 1 is the starting point (before any turn).
    verts[1] = { x = x, y = y, keypoint = false }
    local cx, cy = x, y
    local h = heading_deg
    for i = 1, M.EDGE_COUNT do
        h = h + M.TURNS[i]
        local rad = math.rad(h)
        cx = cx + edge_length * math.cos(rad)
        cy = cy + edge_length * math.sin(rad)
        local is_kp = false
        for _, kp in ipairs(M.KEYPOINTS) do
            if kp == i then is_kp = true; break end
        end
        verts[i + 1] = { x = cx, y = cy, keypoint = is_kp }
    end
    -- The polygon must close: vertex 14+1 = vertex 1. The user's
    -- turn sequence guarantees this algebraically (sum is 360°) but
    -- we don't assert it here -- the user said this is "ground
    -- truth" and should not be tested.
    return verts
end

-- Convenience: extract just the keypoint vertices from a walk.
function M.keypoint_positions(verts)
    local out = {}
    for _, kp in ipairs(M.KEYPOINTS) do
        out[#out + 1] = verts[kp + 1]  -- verts are 1-indexed
    end
    return out
end

-- Place a Spectre such that two of its keypoints coincide with two
-- specified target keypoints. This is the core operation of the
-- Appendix A construction: given an anchor Spectre (or anchor
-- "prism") with two of its keypoints at known positions, place a
-- new Spectre so its corresponding two keypoints fall at those same
-- positions.
--
-- Args:
--   anchor_pts  : list of 2 vertex tables {x, y} = the target
--                 keypoint positions of the already-placed tile.
--   my_kp_idx   : list of 2 indices into M.KEYPOINTS, naming which
--                 of MY keypoints should coincide with the
--                 corresponding anchor_pts. So my_kp_idx[1] is the
--                 keypoint that will land at anchor_pts[1].
--   edge_length : the edge length of the new Spectre (defaults to 1).
--
-- Returns { x, y, heading_deg } giving the starting vertex and
-- initial heading of the new Spectre.
function M.snap_to_keypoints(anchor_pts, my_kp_idx, edge_length)
    if edge_length == nil then edge_length = 1 end
    -- Compute the positions of MY keypoints relative to MY starting
    -- vertex and initial heading. Walk a Spectre with unit scale and
    -- heading 0 starting at origin; the keypoint positions in that
    -- frame are constants.
    local ref = M.walk(0, 0, 0, edge_length)
    local my_pts = {
        ref[my_kp_idx[1] + 1],  -- my keypoint 1
        ref[my_kp_idx[2] + 1],  -- my keypoint 2
    }
    -- We need a rigid motion (rotation + translation) that maps
    -- my_pts[1] -> anchor_pts[1] and my_pts[2] -> anchor_pts[2].
    -- Solve for rotation angle and translation.
    local p1, p2 = my_pts[1], my_pts[2]
    local q1, q2 = anchor_pts[1], anchor_pts[2]
    local dx1, dy1 = p2.x - p1.x, p2.y - p1.y
    local dx2, dy2 = q2.x - q1.x, q2.y - q1.y
    local a1 = math.atan2(dy1, dx1)
    local a2 = math.atan2(dy2, dx2)
    local rotation = a2 - a1
    local cos_r, sin_r = math.cos(rotation), math.sin(rotation)
    -- Translate: take p1 -> q1.
    local px, py = p1.x, p1.y
    local new_x = q1.x - (px * cos_r - py * sin_r)
    local new_y = q1.y - (px * sin_r + py * cos_r)
    -- Apply the same rotation to heading 0 to get the new initial
    -- heading. Heading is in degrees.
    local new_heading = math.deg(rotation)
    return { x = new_x, y = new_y, heading = new_heading }
end

return M
