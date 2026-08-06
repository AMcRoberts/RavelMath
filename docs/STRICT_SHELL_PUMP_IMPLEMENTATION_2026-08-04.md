# Concrete strict-shell-pump operation

The abstract `StrictShellPump` seam now has a reusable C++ certificate operation:

- `include/ravel/proof/strict_shell_pump.hpp`
- `tests/strict_shell_pump_test.cpp`
- Make target: `strict_shell_pump_test`

## Inputs

The operation consumes:

1. a replayable closed n-bonacci carry cycle;
2. a cyclic continuation-controller plant;
3. a replayable closing controller run over the source digit word.

## Derived evidence

`certify_strict_shell_pump` derives rather than accepts:

- every translation defect from consecutive controller states;
- every transported digit from source digit plus translation defect;
- the entire transported state path;
- recurrence replay for the translation and transported paths;
- transported-cycle closure;
- source and lifted shell radii;
- outward face alignment at the selected cycle basepoint.

The certificate is valid exactly when the adjusted digits are admissible, both
cycles replay and close, and the aligned transported cycle has strictly larger
radius.

Failures carry an explicit `unsupported_reason`; the operation does not insert
raw proof text or silently assume the missing property.

## Synthesis

`synthesize_strict_shell_pump` searches cyclic controller fixed points for the
source word and returns the first fully replayed outward certificate. This
reuses the existing whole-word fixed-point and path-reconstruction machinery.

## Lean handoff

`lean/cyclic_controller_pumping.lean` now includes
`StrictShellPumpWitness` and `cyclicStrictShellPump_of_witness`, which convert a
witness-producing engine operation into the abstract `CyclicStrictShellPump`
proposition.

## Remaining universal obligation

This implementation closes the *operation* seam but does not assert a false
universal theorem. The campaign must still prove that every realized outer
recurrent cycle has at least one face-aligned cyclic controller fixed point.
When synthesis cannot find one, it reports the unsupported reason and leaves
that controller-existence obligation visible.
