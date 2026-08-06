local M = {}

function M.main(_ravel)
  local tree = require("ravel.proof.nbonacci_universal_strategy_tree")
  local schema = require("ravel.proof.nbonacci_charpoly_schema")
  assert(tree.schema_version == 1)
  assert(type(tree.obligations) == "table")
  local schema_ids = {}
  for _, o in ipairs(schema.obligations) do schema_ids[o.id] = true end
  local course_ids = {}
  local course_count = 0
  for obligation_id, entry in pairs(tree.obligations) do
    assert(schema_ids[obligation_id], "strategy for unknown obligation: " .. obligation_id)
    assert(type(entry.objective) == "string" and #entry.objective > 0)
    assert(type(entry.courses) == "table" and #entry.courses >= 2,
           obligation_id .. " must have at least two courses")
    for _, c in ipairs(entry.courses) do
      assert(type(c.id) == "string" and not course_ids[c.id], "duplicate course id: " .. tostring(c.id))
      course_ids[c.id] = true
      course_count = course_count + 1
      assert(type(c.method) == "string" and #c.method > 0)
      assert(type(c.success) == "string" and #c.success > 0)
      assert(type(c.next_on_failure) == "table")
    end
  end
  for id in pairs(schema_ids) do
    assert(tree.obligations[id], "missing strategy tree for obligation: " .. id)
  end
  for _, entry in pairs(tree.obligations) do
    for _, c in ipairs(entry.courses) do
      for _, next_id in ipairs(c.next_on_failure) do
        assert(course_ids[next_id], c.id .. " references missing failure pivot " .. next_id)
      end
    end
  end
  assert(course_count >= 30, "strategy tree is too shallow")
  print("n-bonacci universal strategy tree: PASS (" .. course_count .. " courses)")
end

return M
