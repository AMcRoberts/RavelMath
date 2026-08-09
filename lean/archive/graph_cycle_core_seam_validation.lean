import Mathlib.Tactic

namespace RavelGenerated

/-- The edge-count closed form for the installed graph attractor model:
    an n-cycle plus one transient feeder edge. -/
def cycleWithFeederOutdegrees (n : ℕ) : List ℕ :=
  List.replicate n 1 ++ [1]

theorem cycleWithFeederOutdegrees_length (n : ℕ) :
    (cycleWithFeederOutdegrees n).length = n + 1 := by
  simp [cycleWithFeederOutdegrees]

theorem cycleWithFeeder_edge_count (n : ℕ) :
    (cycleWithFeederOutdegrees n).sum = n + 1 := by
  simp [cycleWithFeederOutdegrees]

/-- Abstract seam theorem: once a graph certificate identifies an n-cycle core
    and one feeder edge, the total directed edge count is forced. -/
theorem edge_count_of_cycle_core_and_feeder
    (n coreEdges feederEdges totalEdges : ℕ)
    (hcore : coreEdges = n)
    (hfeeder : feederEdges = 1)
    (htotal : totalEdges = coreEdges + feederEdges) :
    totalEdges = n + 1 := by
  omega

end RavelGenerated
