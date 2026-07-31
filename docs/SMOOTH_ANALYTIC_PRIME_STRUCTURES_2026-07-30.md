# Smooth analytic prime-indicating structures: what's derived and what's worth torturing

Status: exploratory research vector, parked 2026-07-30. Not part of the
active Class-II Pisot program; recorded so a future session (or a later
point in this one) can pick it up from a precise starting point rather
than reconstructing it from scratch.

## Origin

AM shared a family of Desmos-style constructions (tanh-based pulse
trains, a `d(x,d_3)` tooth-excluding wheel comb, a counting sum, and
`arcsin(sin(...))` triangle-wave pieces) exploring smooth, Gibbs-free
analogues of classical discrete primality-detecting constructions
(sieve-of-Eratosthenes-style wheels, Willans' formula). This document
records what was derived and verified in that exploration, and lays
out concrete "torture" directions for later. Everything in §1-2 is
worked derivation, checked directly; §5 is unexplored.

## 1. The core building block and its derivation

Key identity, verified directly:

```text
1 - 2/(1+e^z) = (e^z-1)/(e^z+1) = tanh(z/2).
```

So every `1 - 2/(1+e^{...})` factor in the shared equations is
literally `tanh` of half the exponent. This is the atom everything
else is built from.

`tanh(k*z)` saturates to `sign(z)` as `k -> infinity`, and -- this is
the point AM's language ("approaches squareness roundly ... rather
than the wackiness that averages") was precisely tracking -- it does
so with **no Gibbs overshoot at any finite k**, because `|tanh| < 1`
always: there is no room for a saturating, bounded, monotonic
nonlinearity to overshoot a target amplitude it can structurally never
exceed. This is a fundamentally different mechanism from a truncated
Fourier/linear approximation to a jump, where Gibbs' ~9% overshoot is
a fixed, non-vanishing feature of the *linear/spectral truncation*
mechanism itself, not of "approximating a jump" in general.

## 2. The multiplication-table sieve

The shared limit

```text
J(x) = lim_{k->inf} ( prod_{v=0}^k prod_{n=0}^k
         tanh( k( x/(v+2) - (n+2) ) ) )^2
```

is, via the identity above, a smooth encoding of the
multiplication-table characterization of compositeness: `x` is
composite iff `x = (v+2)(n+2)` for some tested pair, and any such hit
sends its own factor to exactly `tanh(0)=0`, zeroing the whole
product.

**Derived simplification.** The inner loop over `n` -- testing whether
`x/(v+2)` equals any of the finitely many enumerated integers
`2,...,k+2` -- is redundant with what periodicity already supplies for
free: `sin(pi*y)=0` exactly when `y` is an integer, for *every*
integer, not a finite enumerated subset. So

```text
J(x)  ~>  prod_{v=0}^k [ tanh( k*sin(pi * x/(v+2)) ) ]^2
```

collapses the double loop to a single loop: one factor per candidate
divisor, each factor itself the single primitive
`[tanh(k*sin(.))]^2`, testing divisibility by `v+2` against *every*
integer quotient at once rather than an enumerated range. This
removes one whole dimension of finite windowing (the quotient side)
for free; the remaining loop, over candidate divisors, is irreducible
for the reason in section 4.

**Derived tooth-nipping mechanism.** To exclude a specific point from
a comb of zeros (e.g. the trivial zero at `y=0` for
`tanh(k*sin(pi*y/d))^2`) without disturbing the other teeth, use
*addition*, not multiplication -- multiplying by anything already zero
there can't un-zero it. A fast-decaying localized bump added at the
point to be excluded works:

```text
g~(y) = tanh(k*sin(pi*y/d))^2 + e^{-k^2 y^2}.
```

This is the same move `d(x,d_3)`'s explicit subtract-then-add-2 was
doing by hand for one specific tooth, generalized to a clean one-line
addition.

**Derived domain gate.** `(1+tanh(k*x))/2` is a smooth one-sided gate,
`->0` for `x<0`, `->1` for `x>0` -- a single extra factor of the same
primitive suffices to restrict the whole construction to the domain
of interest without special-casing negative or zero `x`.

## 3. What's genuinely nice about this family (verified, not speculative)

- **Stability at every finite k.** Unlike the naive infinite products
  discussed earlier in the session (e.g. `prod sin(pi*n/k)` extended
  to `k -> inf`, which collapses to zero identically because its
  factors tend to 0 rather than 1), this family is well-defined,
  bounded, and smooth at *every* finite `k`, everywhere on its domain
  -- no blow-ups, no premature collapse. The `tanh` saturation
  structurally forbids it.
- **Analyticity.** `tanh` and `sin` are entire/meromorphic in a
  complex variable. For fixed finite `k`, the whole construction is a
  genuine meromorphic function of complex `x` -- something no
  floor/factorial-based construction (Willans' actual formula, or any
  Wilson's-theorem-based indicator) can ever be, since those aren't
  even continuous.
- **Graded local information.** Near a prime the finite-k function
  sits flat on a plateau (small derivatives); near a composite it's
  mid-dive through a genuine zero, and the local slope/curvature
  carries continuous information a hard 0/1 indicator discards
  entirely. This is what would make gradient-based exploration of the
  construction possible, in the same spirit as this project's existing
  smooth-relaxation search over Pisot candidate matrices
  (`docs/DIRECTION_AND_OPEN_THREADS.md`, item B2).
- **Fourier tail control.** Smoothness bounds Fourier-coefficient
  decay (faster than any polynomial rate for `C^inf`; exponential for
  real-analytic), unlike a jump indicator's slowly-decaying,
  Gibbs-afflicted spectrum.

**Honest limit.** All of the above lives in the finite-`k` family. The
idealized limit `k -> infinity` throws it all away and returns an
ordinary hard indicator with none of these properties. The
interesting mathematics, if there is any beyond "a nicer way to build
wheel sieves," lives in studying the *family*, not its limit. And
regardless of smoothness, the combinatorial content is unchanged from
ordinary wheel factorization: the divisor loop is still finite, still
needs to grow with `x` at the same rate ordinary trial division does
(section 4 is about why this can't be helped, not a defect in the
construction).

## 4. Why the outer (divisor) loop can't collapse the same way the inner one did

Primality is not a single-period property. "Is `x` a multiple of `d`"
is; "is `x` composite" is a disjunction over every candidate `d` from
2 up to `~sqrt(x)`, each a genuinely different modulus. No single
fixed-frequency wave can encode membership in that many different
residue classes at once -- this is the same fact, restated, as "the
primes are not almost periodic," established earlier in this session's
conversation. So the divisor loop is irreducible content, not
unnecessary complication; what got simplified was the redundant
enumeration inside a *single* modulus, not the number of moduli that
must be tested.

## 5. Torture directions (unexplored -- this is the actual parked work)

1. **Push it into the complex plane.** For fixed finite `k`,
   plot/derive the pole and zero locations of the collapsed
   single-loop `J`-analogue as a meromorphic function of complex `x`.
   `tanh(w)` has simple poles at `w = i*pi*(n+1/2)`; work out where
   those land for the rescaled arguments here. If a sane contour can
   be found enclosing a real interval `[2,N]` without crossing a pole,
   the argument principle -- `(1/2*pi*i) * oint f'/f dz` counts zeros
   minus poles enclosed -- would hand a *contour-integral* count of
   composites in that range, structurally the same move as Perron's
   formula / the explicit formula for `psi(x)` in real analytic number
   theory. Unchecked: whether the poles stay away from any useful
   contour, and how the `k -> infinity` limit interacts with a contour
   argument (do poles march toward the real axis, cancel, or stay
   clear).
2. **Ultrametric / p-adic structure.** AM's own suggestion; not yet
   given a precise formulation. If there's a meaningful valuation to
   extract from this construction's behavior (rate of approach to a
   zero, perhaps, viewed through a `p`-adic rather than real lens),
   this project already has the local-field/valuation machinery
   (`include/adelic/padic.hpp`, `local_field.hpp`) that could be
   repurposed to measure it, once "measuring its ultrametric" is made
   concrete enough to compute.
3. **Projection of something higher-dimensional.** The honest existing
   analog in the literature is Kurasov & Sarnak's Fourier-quasicrystal
   construction from the zeros of `zeta(s)` (and Dyson's original
   remark that those zeros "look like" a quasicrystal) -- a genuine
   cut-and-project-style object, but built from the zeta zeros, not
   from a wheel-sieve. Whether *this* construction is itself a
   shadow/projection of something in that spirit, or a structurally
   unrelated smoothing trick, is open; this project's own
   cut-and-project machinery (`docs/CUT_AND_PROJECT_PROOF_DIRECTION.md`)
   is the natural vocabulary to pose the question precisely in, if
   it's worth posing at all.
4. **Spectral decomposition at finite k.** Actually compute the
   Fourier coefficients of the finite-`k` collapsed construction over
   some range and look at what decays fast vs. slow -- a cheap,
   concrete first computational step before attempting 1-3.

5. **The most concrete bridge: this project's own stepped-hyperplane
   predicate is the same shape.** The signed stepped-hyperplane
   acceptance test used throughout the Class-II contact/corona
   machinery, `0 <= <x,v> < v_j` (see
   `docs/STEPPED_HYPERPLANE_COMPARTMENTS_AND_WALLS.md`,
   `math/include/math/in_h_sigma.hpp`), is exactly a half-open
   interval membership test -- the identical shape as section 1's
   very first tanh window primitive
   (`-tanh(k(x+pi/2))*tanh(k(x-pi/2))+2`, a smooth window on
   `(-pi/2,pi/2)`), adapted to a half-open rather than open interval.
   This is not speculative in the way 1-3 are: the sharp acceptance
   test this project already runs on is structurally the same object
   this section's primitive smooths. A smooth relaxation of window
   membership -- not just of the matrix-entry objective already used
   in the smooth-relaxation search (`docs/DIRECTION_AND_OPEN_THREADS.md`
   item B2) -- would let near-wall configurations be treated
   continuously (a graded "how close to the wall" value) rather than
   as a sharp accept/reject, which is exactly the kind of information
   sections 3's audit-rule discussion of wall/compartment behavior
   currently has to reason about by cases. Whether this is useful for
   the actual open occurrence/exhaustion seam, or just a nice
   observation, is unexplored -- but it is the one item on this list
   that connects directly to the active program rather than to a
   separate research question.

None of this is claimed to work. It's recorded so that attempting it
later starts from the actual derivation above rather than from
scratch.
