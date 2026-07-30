-- lua/ravel/substitution_rule.lua
--
-- Substitution rule acting on a finite alphabet, plus the Parikh-vector
-- balanced-pair helper that powers test_tribonacci_balanced_pair.lua.
-- This is the same algorithm as
-- include/ravel/substitution_rule.hpp in the C++ project; see
-- docs/THEOREM_STATUS.md section 2 for the
-- limitations (this is NOT the Hollander-Solomyak / Barge-Diamond
-- certification algorithm).

local M = {}

local SubstitutionRule = {}
SubstitutionRule.__index = SubstitutionRule

function SubstitutionRule.new(sigma)
    assert(#sigma > 0, "substitution rule must have non-empty alphabet")
    return setmetatable({ sigma = sigma }, SubstitutionRule)
end

-- Re-encode a rule given with letters 'a'..'z' to alphabet indices 0..n-1.
-- Out-of-alphabet characters map to 0 (matches C++ behaviour).
function M.reencode(rule)
    local n = #rule
    local out = {}
    for j = 1, n do
        local s = rule[j] or ""
        local t = {}
        for i = 1, #s do
            local byte = string.byte(s, i)
            local v = byte - string.byte("a")
            if v < 0 or v >= n then
                t[#t + 1] = string.char(0)
            else
                t[#t + 1] = string.char(v)
            end
        end
        out[j] = table.concat(t)
    end
    return out
end

function SubstitutionRule:alphabet_size() return #self.sigma end

function SubstitutionRule:image(i) return self.sigma[i] end

function SubstitutionRule:apply_once(word)
    local out = {}
    for i = 1, #word do
        local c = word:sub(i, i)
        local b = string.byte(c)
        -- b is in 0..255.  Map to a 1-based Lua index into sigma.
        -- Sigma images were encoded with bytes 0..n-1 where byte 0
        -- corresponds to letter 'a' (alphabet index 0).  The C++
        -- apply_once indexes sigma_[b] directly, which is the same
        -- offset we replicate here with `b + 1`.
        local idx = b + 1
        if idx >= 1 and idx <= #self.sigma then
            out[#out + 1] = self.sigma[idx]
        else
            out[#out + 1] = c
        end
    end
    return table.concat(out)
end

function SubstitutionRule:apply_level(letter, level)
    local s = letter
    for _ = 1, level do s = self:apply_once(s) end
    return s
end

function SubstitutionRule:apply_to_alphabet(level)
    local out = {}
    for i = 1, self:alphabet_size() do
        out[i] = self:apply_level(string.char(i - 1), level)
    end
    return out
end

function SubstitutionRule:matrix()
    local n = self:alphabet_size()
    local M = {}
    for i = 1, n do M[i] = {}; for j = 1, n do M[i][j] = 0 end end
    for j = 1, n do
        local s = self.sigma[j]
        for k = 1, #s do
            local idx = string.byte(s, k) + 1  -- 1-indexed
            M[idx][j] = M[idx][j] + 1
        end
    end
    return M
end

-- Compute |sigma^k(j)| for each j, 0 <= k <= max_level, exactly via M^k.
function SubstitutionRule:length_table(max_level)
    local n = self:alphabet_size()
    local M = self:matrix()
    local result = {}
    result[1] = {}  -- level 0
    for j = 1, n do result[1][j] = 1 end
    local Mk = M
    for k = 1, max_level do
        local colsums = {}
        for j = 1, n do colsums[j] = 0 end
        for i = 1, n do
            for j = 1, n do colsums[j] = colsums[j] + Mk[i][j] end
        end
        result[k + 1] = colsums
        local next = {}
        for i = 1, n do
            next[i] = {}
            for j = 1, n do next[i][j] = 0 end
        end
        for i = 1, n do
            for kk = 1, n do
                for j = 1, n do
                    next[i][j] = next[i][j] + Mk[i][kk] * Mk[kk][j]
                end
            end
        end
        Mk = next
    end
    return result
end

M.SubstitutionRule = SubstitutionRule

-- Balanced-pair helpers (Parikh vector only)
function M.letter_counts(word, alphabet_size)
    local counts = {}
    for i = 1, alphabet_size do counts[i] = 0 end
    for i = 1, #word do
        local idx = string.byte(word, i) + 1
        assert(idx >= 1 and idx <= alphabet_size,
               "word contains a symbol outside the alphabet")
        counts[idx] = counts[idx] + 1
    end
    return counts
end

function M.is_balanced(pair, alphabet_size)
    local cl = M.letter_counts(pair.left, alphabet_size)
    local cr = M.letter_counts(pair.right, alphabet_size)
    for i = 1, alphabet_size do
        if cl[i] ~= cr[i] then return false end
    end
    return true
end

function M.is_coincidence(pair)
    return #pair.left == 1 and #pair.right == 1
       and pair.left == pair.right
end

local function prefix_counts(word, alphabet_size)
    local n = #word
    local pref = {}
    pref[0] = {}
    for i = 1, alphabet_size do pref[0][i] = 0 end
    for k = 1, n do
        pref[k] = {}
        for i = 1, alphabet_size do pref[k][i] = pref[k - 1][i] end
        local idx = string.byte(word, k) + 1
        assert(idx >= 1 and idx <= alphabet_size,
               "word contains a symbol outside the alphabet")
        pref[k][idx] = pref[k][idx] + 1
    end
    return pref
end

function M.split_balanced_pair(pair, alphabet_size)
    assert(M.is_balanced(pair, alphabet_size),
           "cannot split an unbalanced pair")
    local left_pref  = prefix_counts(pair.left, alphabet_size)
    local right_pref = prefix_counts(pair.right, alphabet_size)
    local result = {}
    local left_begin, right_begin = 0, 0
    while left_begin < #pair.left or right_begin < #pair.right do
        local found = false
        local best_left, best_right, best_sum = 0, 0, 0
        for left_end = left_begin + 1, #pair.left do
            for right_end = right_begin + 1, #pair.right do
                local equal = true
                for s = 1, alphabet_size do
                    if left_pref[left_end][s] - left_pref[left_begin][s]
                    ~= right_pref[right_end][s] - right_pref[right_begin][s] then
                        equal = false; break
                    end
                end
                if equal then
                    local sum = left_end + right_end
                    if not found or sum < best_sum
                    or (sum == best_sum and left_end < best_left) then
                        found = true
                        best_left, best_right, best_sum = left_end, right_end, sum
                    end
                end
            end
        end
        assert(found, "balanced pair has no balanced factorisation")
        result[#result + 1] = {
            left = pair.left:sub(left_begin + 1, best_left),
            right = pair.right:sub(right_begin + 1, best_right),
        }
        left_begin, right_begin = best_left, best_right
    end
    return result
end

function M.refine_balanced_pair(rule, pair)
    assert(M.is_balanced(pair, rule:alphabet_size()),
           "cannot refine an unbalanced pair")
    local image = {
        left = rule:apply_once(pair.left),
        right = rule:apply_once(pair.right),
    }
    return M.split_balanced_pair(image, rule:alphabet_size())
end

function M.refine_balanced_pairs(rule, pairs)
    local out = {}
    for _, pair in ipairs(pairs) do
        if M.is_coincidence(pair) then
            out[#out + 1] = pair
        else
            local refined = M.refine_balanced_pair(rule, pair)
            for _, r in ipairs(refined) do out[#out + 1] = r end
        end
    end
    return out
end

return M
