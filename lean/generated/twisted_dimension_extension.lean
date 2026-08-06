import Mathlib

namespace RavelPlayground

/-- A twisted extension carries a base edge to an upper path.  The sheet
    correction eta is a 0-cochain, so upper path transport is lower edge
    transport corrected at the two endpoints. -/
theorem twisted_extension_transport
    (lowerFiber upperFiber etaS etaT : Bool)
    (hsource : upperFiber = (etaS != lowerFiber))
    (targetLower targetUpper : Bool)
    (htarget : targetUpper = (etaT != targetLower)) :
    (upperFiber != targetUpper) =
      (etaS != (lowerFiber != targetLower) != etaT) := by
  subst upperFiber
  subst targetUpper
  cases etaS <;> cases etaT <;>
    cases lowerFiber <;> cases targetLower <;> decide

/-- Replacing each lower edge by a nonempty upper path preserves cocycle
    composition because intermediate endpoint fibers cancel. -/
theorem xor_path_substitution
    (a b c : Bool) :
    (a != b) != (b != c) = (a != c) := by
  cases a <;> cases b <;> cases c <;> decide

end RavelPlayground
