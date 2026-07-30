# Global catalogue occurrence and exhaustion

Status: active stitching theorem after closure of the center contact
base, 2026-07-29.

## What is now available

The center contact base is no longer a hypothesis:

```text
9 D_cont seeds -> 16 pre-contact states -> 14 contact states
```

for every `a>=2`.  The reverse certificate derives all raw symbolic
prefix families, classifies all 255 bounded window cases with zero
unresolved, and counts `6a+21` restricted branches.

For fixed-light neighbor 2, the local layer obligations are also
separated and individually exact on their stated parameter domains:

1. center-interface support has an exact contact-base chain and
   `7+4` bounded composition witnesses;
2. stable raw-corona inclusion and window exhaustion;
3. stable Red ranks, including later-rank predecessor maps;
4. penultimate repartition, raw inclusion, and all three Red ranks;
5. terminal affine edit, raw inclusion/window exhaustion, and rank
   edit;
6. repeated-terminal raw exhaustion and the six Red ranks;
7. explicit Red bridges from every correction layer into a surviving
   correction or center state.

The enrolled test now makes the first dependency explicit:
`ClassIINeighbor2CenterInterfaceCertificate::exact()` requires the
universal center-contact certificate, not merely literal containment
in a hand-written fourteen-state set.

## What “stitching” still means

Lean file `class_ii_global_round_partition.lean` now proves that for
every `a>=7` the five domains below are disjoint and exhaustive for
`1<=r<=a+1`.  Its theorem `class_ii_global_round_stitch` also proves
the total logical router: any round proposition follows globally once
it is supplied on the five displayed domains.  The remaining
mathematical induction object must state:

```text
B_r(a) = the displayed pre-Red catalogue at round r
A_r(a) = the displayed post-Red catalogue at round r
```

and prove, with every exceptional range explicit,

```text
raw_corona(A_{r-1}(a), +/-C(a)) = B_r(a)
Red(B_r(a)) = A_r(a).
```

The proof must route rounds into:

- bounded base rounds;
- stable interior rounds;
- penultimate;
- terminal;
- repeated terminal/fixed point.

The existing certificates prove the branch-local equalities used in
the stable and endpoint cases.  The total phase partition and generic
induction router are now kernel checked.  The remaining seam is to
instantiate their proposition with the actual `A_r/B_r` definitions,
promote the four bounded base rounds from support/composition
certificates to full raw-corona/Red equalities, and then feed all five
equalities to the router.  Small parameters below the stable threshold
must remain a separate finite table instead of pretending the `a>=7`
grammar covers them.

The C++ side now names these schematic objects for every closed
non-base phase:

- `class_ii_neighbor2_nonbase_pre_red_catalogue(a,r)` is `B_r`;
- `class_ii_neighbor2_nonbase_pruning_ranks(a,r)` is the ordered Red
  partition;
- `class_ii_neighbor2_nonbase_post_red_catalogue(a,r)` is `A_r`.

The dispatcher is total on stable, penultimate, terminal, and repeated
rounds and deliberately rejects base rounds. The enrolled test checks
429 instantiated non-base catalogues for `7<=a<=32`: every rank lies
in `B_r`, ranks are disjoint, and reinserting all removed ranks into
`A_r` reconstructs `B_r`. These are named definitions and structural
regression checks; the universal local certificates remain the
evidence for their transition meaning.

### The four base premises, explicitly

The word “base” must not collapse four different statements:

| round | displayed neighbor-2 correction | already closed | still needed for the global equality |
|---:|---|---|---|
| 1 | mirror-closed `E_1`, 22 states | universal center contact base; literal signed-contact partition; window validity | reverse inclusion for the neighbor signed-contact set |
| 2 | fixed `E_2`, 25 states | center/correction positive composition witnesses and explicit Red successors/bridges | full raw-corona reverse inclusion and full Red exclusion |
| 3 | 24 fixed roles plus the first tip | positive center-interface/correction witnesses, window validity, Red retention | full raw-corona reverse inclusion and full Red exclusion |
| 4 | 24 fixed roles plus the affine tip | interior coefficientwise positive transport and Red retention | connect the bounded round-four grammar to the universal stable reverse/exclusion theorem whose stated domain begins at round five |

These are occurrence/exhaustion gaps, not missing endpoint-validity or
survival lemmas. In particular, the `7+4` center-interface witnesses
do not prove that no other raw state occurs, and a positive Red bridge
does not prove that every undesired state is pruned.

The exact center trace provides a useful finite target without proving
the neighbor transition: for every sampled `a>=4`, its post-Red base
sizes are `28,47,68,88` and its raw sizes at rounds two through four
are `104,167,238` once `a>=5`; the collision at `a=4` gives raw
round-four size `241`. The neighbor transport formula then proposes
post-Red corrections of sizes `22,25,25,25` (the last two are the
24 fixed roles plus one tip). These counts are discovery data and
implementation targets; the missing reverse/exclusion proofs must
identify the literal sets, not merely reproduce their cardinalities.

The independent exact trace comparison confirms the proposed
neighbor post-Red base sizes for `a=7,8`:

```text
round             1   2   3    4
center A_r       28  47  68   88
neighbor A_r     50  72  93  113
literal E_r      22  25  25   25
```

Every center layer is literally contained and the set difference is
the displayed correction. This pins down the four `A_r` targets. It
remains finite discovery evidence until the corresponding raw
`B_r` reverse inclusions and Red exclusions are proved universally.

## Recurrent exhaustion after layer equality

Full layer equality proves occurrence/exhaustion of boundary *states*.
It does not by itself prove that the displayed recurrent catalogues
are exactly all recurrent SCCs.  That second step needs:

1. a partition of the fixed-point catalogue into the displayed core,
   regular-shell, special-shell, and transient sets;
2. universal strong connectivity of every displayed recurrent block
   (already certified for catalogue matrices);
3. at least one directed escape from every transient block toward a
   later block;
4. absence of return paths from recurrent blocks to earlier transient
   strata, most naturally through a monotone rank/round function;
5. absence of edges joining two recurrent blocks that are claimed to
   be distinct SCCs.

Only after these support statements are tied to the globally
identified boundary graph may the universal polynomial/dominance
results be promoted from catalogue theorems to graph theorems.

## Neighbor scope

Neighbor 2 has the detailed layer grammar needed for the stitching
induction.  Neighbors 0 and 1 currently have universal affine core
matrices, shell matrices, polynomials, and irreducibility, but not
comparable complete raw-corona/Red grammars.  Their global occurrence
proof therefore remains a separate, earlier-stage problem.

## Next implementation

Define the four base `A_r/B_r` catalogues and a small
`ClassIINeighbor2GlobalInductionCertificate` that does not recompute
local eliminations. It should consume their exact booleans, use the
now-kernel-checked round router, record the four base-round equalities
and the small-parameter table separately, and report the first missing
local premise rather than a single opaque false flag. The corresponding
theorem document can then distinguish:

- local transition certificates (closed);
- global round induction (next);
- recurrent SCC exhaustion (afterward).
