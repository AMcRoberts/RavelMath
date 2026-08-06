# Source-conditioned defect-splice synthesis

The homogeneous covering-witness library exposed the correct geometric object,
but exact-overlap splicing alone cannot extend a homogeneous orbit: the next
translation window is deterministic.

The generalized splice operation therefore permits a defect only in the
n-bonacci digit-forcing coordinate.  If the base digit is `d` and the splice
defect is `q`, the next translation window is

    t[k+1] = A t[k] + q e_last

and the transported path uses the adjusted digit

    d' = d + q.

The certificate requires `d' ∈ {-1,0,1}`.  Consequently the transported path
remains an exact path of the original n-bonacci digit system; no off-model reset
is hidden at the splice.

For each exact first-return edge, the synthesizer searches boundary translation
windows of radius one, conditioned on the source, the whole digit word, and the
outer-shell geometry.  It emits every successful translation path, every splice
defect, the adjusted digit word, and the transported path.  A separate checker
replays all recurrences and shell conditions.

Exact corpus results:

- n=3, M=2: 298/298 (100%); exact homogeneous=101; defect-spliced=197; delta=1 for every certificate.
- n=3, M=3: 848/848 (100%); exact homogeneous=213; defect-spliced=635; delta=1 for every certificate.
- n=4, M=2: 1774/1774 (100%); exact homogeneous=307; defect-spliced=1467; delta=1 for every certificate.

This is a substantial finite closure result: the old witness horizon is no
longer a limitation, and every audited path length is covered.  It is not yet a
uniform arbitrary-`n`, arbitrary-`M` theorem.

The next seam is now much sharper.  The data suggest a possible universal
radius-one synthesis theorem:

    every exact maximum-shell first return admits a boundary-valued translation
    path with forcing-coordinate defects that keep all adjusted digits admissible.

Proving that requires deriving the translation choice online from the current
base state and remaining return word, or identifying a finite-state strategy
whose transition table is uniform in `M` and preferably in `n`.
