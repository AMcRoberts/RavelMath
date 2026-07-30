# CSY exactness, runtime, and memory boundary

Audited 2026-07-29.

The recurrence and companion-matrix routes compute `beta^n` as exact
`Q(beta)` coefficients. The bounded LRU returns the same exact value;
eviction changes runtime, not arithmetic.

Exact coefficient size grows with `n`. An API returning the expanded
exact value therefore cannot promise memory independent of `n`: its
output alone has growing bit length.

`BetaPowerCache` now enforces two retention limits independently:
maximum entry count and maximum summed numerator/denominator
coefficient bits. If one value exceeds the entire bit budget, it is
returned exactly but not retained. The budget excludes container
overhead and matrix-exponentiation temporaries; it bounds cache
retention, not whole-process RSS.

## Thread-local recurrence window repaired

`pisotContextFor` still retains a thread-local `PisotContext`, but its
recurrence prefix is now limited independently to 64 entries and
1,048,576 stored coefficient bits. A request outside that retained
prefix is computed exactly by companion-matrix exponentiation and is
not stored. Tests exercise exponent 200 and assert both retention
bounds. Thus exponent growth no longer grows the context's power
table; exact output size and matrix temporaries still necessarily grow
with the requested exponent.

The remaining interface work is to make the power provider explicit:

```text
exact recurrence cache   growing; fast sequential access
bounded exact LRU        bounded retained bits; matrix misses
streaming recurrence     O(d) values; sequential positions only
symbolic automaton       finite only after the actual CSY quotient
```

The thread-local map of distinct `PisotPoly` contexts is not itself an
LRU. A workload presenting indefinitely many different polynomials can
still grow that map. Returning a bare context reference currently
prevents safe eviction, so this requires an ownership/API change rather
than pretending the per-context power bound solves it.

Cache engineering also does not repair the mathematical automaton
gap: the current state contains absolute position. The former
unconditional closure flag has been repaired: reaching `max_pos` now
reports truncation (`bfsClosed() == false`), because the zero digit
always supplies a next-position successor. The prototype is an exact
bounded-word recognizer, not yet the finite CSY quotient automaton.

## 2026-07-29 adversarial rerun

The complete 24-section CSY test was rebuilt and rerun after the cache
repair.  It exercises recurrence/matrix agreement, LRU eviction,
capacity saturation, the stored-coefficient bit budget, and an
oversized value that must be returned exactly without retention.  All
checks passed (`0 failures`); the run used about 93,748 KiB peak RSS
and 2.81 seconds on the local validation machine.  That peak includes
the deliberately large bounded-position prototype automata, not just
the 64-entry power prefix.  The result supports the implementation
boundary above; it does not upgrade the prototype into the missing
finite CSY quotient.

An AddressSanitizer/UndefinedBehaviorSanitizer build of the same
24-section test also passes with no reported memory or undefined
behavior error. LeakSanitizer itself cannot run in this managed
environment because the process is traced; rerunning with leak
detection disabled leaves ASan and UBSan active and completes with
zero failures. This is runtime evidence, not a proof of the resource
bounds.

The separate topology test is not evidence for that missing quotient:
its four `sigma_{a,1}` finite-patch homomorphism checks currently fail.
The harness now labels them as expected implementation limitations and
allows any other assertion failure to fail the executable; it no
longer prints an unconditional success after visible failures.
