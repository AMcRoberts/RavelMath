local M = {}

local allowed_kinds = {
  det_expansion = true,
  index_bijection = true,
  triangular_product = true,
  determinant_recurrence = true,
  base_case = true,
  induction = true,
  ring_identity = true,
  theorem = true,
}

local function assert_string(x, label)
  assert(type(x) == "string" and #x > 0, label .. " must be a nonempty string")
end

function M.main(_ravel)
  local schema = require("ravel.proof.nbonacci_charpoly_schema")
  assert(schema.schema_version == 1, "unexpected schema version")
  assert_string(schema.id, "schema.id")
  assert(type(schema.obligations) == "table" and #schema.obligations > 0,
         "schema must contain obligations")

  local by_id = {}
  for _, node in ipairs(schema.obligations) do
    assert_string(node.id, "obligation.id")
    assert(not by_id[node.id], "duplicate obligation id: " .. node.id)
    assert(allowed_kinds[node.kind], "unsupported obligation kind: " .. tostring(node.kind))
    assert_string(node.statement, node.id .. ".statement")
    assert_string(node.validator, node.id .. ".validator")
    assert_string(node.lean_emitter, node.id .. ".lean_emitter")
    assert(type(node.requires) == "table", node.id .. ".requires must be a table")
    by_id[node.id] = node
  end

  for _, node in ipairs(schema.obligations) do
    for _, dep in ipairs(node.requires) do
      assert(by_id[dep], node.id .. " has missing dependency " .. tostring(dep))
      assert(dep ~= node.id, node.id .. " depends on itself")
    end
  end

  local visiting, visited = {}, {}
  local function visit(id)
    assert(not visiting[id], "dependency cycle at " .. id)
    if visited[id] then return end
    visiting[id] = true
    for _, dep in ipairs(by_id[id].requires) do visit(dep) end
    visiting[id] = nil
    visited[id] = true
  end
  for id in pairs(by_id) do visit(id) end

  assert(by_id.universal_theorem, "universal_theorem node missing")
  assert(schema.finite_regression.n_min == 2)
  assert(schema.finite_regression.n_max >= 8)
  print("n-bonacci Lua proof schema: PASS")
end

return M
