-- Declarative proof schema for the universal n-bonacci characteristic
-- polynomial theorem.  This file is data, not a proof and not trusted code.
-- C++ must validate every node before emitting Lean.

return {
  schema_version = 1,
  id = "nbonacci.characteristic_polynomial.universal",
  title = "Universal n-bonacci characteristic-polynomial determinant",

  parameters = {
    { name = "n", sort = "Nat", assumptions = { "2 <= n" } },
  },

  -- Activates latent instrumentation in the mathematical library itself.
  -- The executable is only a trigger; these named pivots are recorded by
  -- matrix construction, minor formation, determinant expansion, and
  -- polynomial normalization as those operations actually execute.
  reflection = {
    enabled = true,
    mode = "observe_and_prove",
    pivots = {
      "matrix.construct",
      "matrix.det.sparse_pivot",
      "matrix.det.first_row",
      "matrix.minor",
      "matrix.det.cofactor",
      "polynomial.geometric_sum",
    },
    outputs = { "debug_trace", "lean_lemma_candidates" },
  },

  symbols = {
    coefficient_ring = "Polynomial Int",
    variable = "X",
  },

  matrices = {
    {
      id = "characteristic_matrix",
      rows = "n",
      cols = "n",
      entry_rule = "nbonacci_companion_characteristic_entry",
    },
    {
      id = "r_matrix",
      rows = "n - 1",
      cols = "n - 1",
      entry_rule = "r_lower_bidiagonal_entry",
    },
    {
      id = "q_matrix",
      rows = "n - 1",
      cols = "n - 1",
      entry_rule = "q_upper_bidiagonal_last_row_entry",
    },
  },

  obligations = {
    {
      id = "characteristic_cofactor_split",
      kind = "det_expansion",
      statement = "det(characteristic_matrix n) = X * det(q_matrix n) + (-1)^n * det(r_matrix n)",
      requires = { "characteristic_minor_maps" },
      validator = "validate_characteristic_cofactor_split",
      lean_emitter = "emit_characteristic_cofactor_split",
    },
    {
      id = "characteristic_minor_maps",
      kind = "index_bijection",
      statement = "the two nonzero cofactors are transported exactly to q_matrix n and r_matrix n",
      requires = {},
      validator = "validate_characteristic_minor_maps",
      lean_emitter = "emit_characteristic_minor_maps",
    },
    {
      id = "r_triangular",
      kind = "triangular_product",
      statement = "det(r_matrix n) = (-1)^(n-1)",
      requires = {},
      validator = "validate_r_triangular",
      lean_emitter = "emit_r_triangular",
    },
    {
      id = "q_minor_reduction",
      kind = "index_bijection",
      statement = "the recurrence cofactor minor of q_matrix (n+1) is q_matrix n",
      requires = {},
      validator = "validate_q_minor_reduction",
      lean_emitter = "emit_q_minor_reduction",
    },
    {
      id = "q_recurrence",
      kind = "determinant_recurrence",
      statement = "qdet(n+1) = X * qdet(n) + 1",
      requires = { "q_minor_reduction" },
      validator = "validate_q_recurrence",
      lean_emitter = "emit_q_recurrence",
    },
    {
      id = "q_base",
      kind = "base_case",
      statement = "qdet(2) = 1 + X",
      requires = {},
      validator = "validate_q_base",
      lean_emitter = "emit_q_base",
    },
    {
      id = "q_closed_form",
      kind = "induction",
      statement = "qdet(n) = sum(k=0..n-1, X^k)",
      requires = { "q_base", "q_recurrence" },
      validator = "validate_q_closed_form_dependencies",
      lean_emitter = "emit_q_closed_form",
    },
    {
      id = "polynomial_closure",
      kind = "ring_identity",
      statement = "X * sum(k=0..n-1, X^k) - 1 = sum(k=0..n-1, X^(k+1)) - 1",
      requires = { "q_closed_form", "r_triangular", "characteristic_cofactor_split" },
      validator = "validate_polynomial_closure",
      lean_emitter = "emit_polynomial_closure",
    },
    {
      id = "universal_theorem",
      kind = "theorem",
      statement = "det(characteristic_matrix n) = nbonacciCharpoly n",
      requires = { "polynomial_closure" },
      validator = "validate_complete_dependency_closure",
      lean_emitter = "emit_universal_theorem",
    },
  },

  finite_regression = {
    n_min = 2,
    n_max = 12,
    checks = {
      "direct determinant equals emitted determinant",
      "minor transport maps preserve entries",
      "q recurrence holds",
      "final polynomial identity holds",
    },
  },

  trust_boundary = {
    "Lua supplies declarative tables only",
    "C++ parses into closed typed variants and rejects unknown fields or node kinds",
    "C++ independently recomputes every claimed transformation",
    "Lean kernel checks the emitted arbitrary-n theorem",
  },
}
