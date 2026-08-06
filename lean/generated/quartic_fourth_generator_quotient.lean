import Mathlib

namespace Ravel

/-- The ordered-prefix refinement has four colours, but the signed-defect
quotient identifies the two neutral colours. -/
inductive QuarticRawColour
| ee | e0 | zeroe | zerozero
  deriving DecidableEq, Repr

inductive SignedDefect
| neutral | plus | minus
  deriving DecidableEq, Repr

def defectQuotient : QuarticRawColour → SignedDefect
| .ee => .neutral
| .e0 => .plus
| .zeroe => .minus
| .zerozero => .neutral

theorem neutral_kernel_pair :
    defectQuotient .ee = defectQuotient .zerozero := by
  rfl

theorem nonneutral_distinct :
    defectQuotient .e0 ≠ defectQuotient .zeroe := by
  decide

end Ravel
