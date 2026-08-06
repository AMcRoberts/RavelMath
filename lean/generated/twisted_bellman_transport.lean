import Mathlib

namespace RavelPlayground

/-- A role embedding transports complete branch profiles by relabelling. -/
def relabelProfile {R U : Type} [Fintype R] [Fintype U]
    [DecidableEq R] [DecidableEq U]
    (ι : R → U) (p : R → Nat) : U → Nat :=
  fun u => ∑ r with ι r = u, p r

/-- If every upper macro-profile is exactly the relabelling of a lower profile,
    Bellman upper propagation commutes with the role embedding.  The Z/2 sheet
    correction does not appear because it changes representatives, not branch
    multiplicities. -/
theorem bellmanUpper_transport
    {R U : Type} [Fintype R] [Fintype U]
    [DecidableEq R] [DecidableEq U]
    (ι : R → U)
    (lower : R → Finset (R → Nat))
    (upper : U → Finset (U → Nat))
    (hprofiles : ∀ r, upper (ι r) = (lower r).image (relabelProfile ι))
    (v : U → Nat) (r : R) :
    (upper (ι r)).sup (fun p => ∑ u, p u * v u) =
      (lower r).sup (fun p => ∑ q, p q * v (ι q)) := by
  rw [hprofiles]
  simp [relabelProfile]

/-- Therefore any strict lower renewal deficit is inherited by the upper
    skew-product macro-system once competitor and core profile families are
    both preserved. -/
theorem strict_renewal_transports
    (lowerCompetitor lowerCore upperCompetitor upperCore : Nat)
    (hc : lowerCompetitor = upperCompetitor)
    (hk : lowerCore = upperCore)
    (hstrict : lowerCompetitor < lowerCore) :
    upperCompetitor < upperCore := by
  omega

end RavelPlayground
