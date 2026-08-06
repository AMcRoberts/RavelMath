import Mathlib

namespace RavelReflectiveBoundaryGrammar

variable {α : Type}

/-- Shift left and append one new boundary element. -/
def shiftAppend (xs : List α) (incoming : α) : List α := xs.drop 1 ++ [incoming]

/-- Ordered left boundary queue of depth `d`. -/
def leftQueue (d : Nat) (xs : List α) : List α := xs.take d

/-- Ordered right boundary queue, nearest incoming boundary first. -/
def rightQueue (d : Nat) (xs : List α) : List α := xs.reverse.take d

/-- The executable queue operation is literally a left shift followed by one insertion. -/
theorem shiftAppend_cons {x : α} {xs : List α} (incoming : α) :
    shiftAppend (x :: xs) incoming = xs ++ [incoming] := by
  simp [shiftAppend]

/-- The newly appended value is the last value of every nonempty shifted queue. -/
theorem getLast_shiftAppend {x : α} {xs : List α} (incoming : α) :
    (shiftAppend (x :: xs) incoming).getLast? = some incoming := by
  simp [shiftAppend]

/-- The first element after shifting is the old second element. -/
theorem head_shiftAppend {x y : α} {xs : List α} (incoming : α) :
    (shiftAppend (x :: y :: xs) incoming).head? = some y := by
  simp [shiftAppend]

/-- Repeating the executable operation gives the grammar induction over words. -/
def run : List α → List α → List α
  | state, [] => state
  | state, a :: word => run (shiftAppend state a) word

@[simp] theorem run_nil (state : List α) : run state [] = state := rfl

@[simp] theorem run_cons (state : List α) (a : α) (word : List α) :
    run state (a :: word) = run (shiftAppend state a) word := rfl

end RavelReflectiveBoundaryGrammar
