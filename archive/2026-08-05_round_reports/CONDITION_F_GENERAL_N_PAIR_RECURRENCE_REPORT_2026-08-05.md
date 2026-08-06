# Condition-F general-n pair recurrence — round 73

## Result

The canonical Condition-F pair has now been separated into the same two layers
that closed the earlier universal-n characteristic proof.

1. **Voltage/Fourier layer.**  For a cyclic fibre, every character sector is
   entrywise modulus-dominated by the untwisted channel-count operator.  Q/R
   type and voltage are independent: larger fibres may twist either generator.
   Hence all holonomy sectors are spectrally bounded by the untwisted Q+R
   sector before the dimension recurrence is considered.

2. **Parent-grammar layer.**  Under D -> D+1 every old parent channel persists.
   The only new parent choice is terminal parent D above inner letter 0.  The
   Q/R classifier and signed residual defect are therefore stable on the old
   principal block and explicit on the new boundary.

3. **Boundary-interface layer.**  The complete terminal-parent correction has
   exactly three families:

   - balanced corner fan over inner pair (0,0): 2D+1 channels;
   - positive residual strip over (0,j), j>0: D-1 channels;
   - negative residual strip over (i,0), i>0: D-1 channels.

   There are no positive-positive boundary channels.  Thus the growing
   transfer pair is a transported old block plus a three-type interface of
   total size 4D-1.  This is the transfer-operator analogue of the old sparse
   q/r cofactor split.

## New operations

- `derive_condition_f_cyclic_voltage_fourier`
- `derive_condition_f_parent_pair_recurrence`
- `derive_condition_f_local_channel_recurrence`
- `derive_condition_f_pair_universal_reduction`
- `derive_condition_f_boundary_interface`

## Validation

The parent and local recurrences were checked through dimensions 128 and 64,
respectively; the boundary formula through dimension 256.  Existing prefix,
twisted quotient, Fourier, component-maximality, finite quotient, terminal
reduction, and universal-dominance regressions pass.

An exact n=8 arithmetic-hull run was attempted as a new-dimension check but did
not finish within the bounded execution window.  No conclusion was inferred
from that timeout.

## Remaining theorem

All twist and matrix-color ambiguity is now gone.  The sole universal spectral
problem is the untwisted role-adjacency boundary extension:

> prove that the three terminal-parent interface families cannot create an
> assembled recurrent component with Perron root above the transported
> predicted-core block.

The likely closure is a phase-refined three-interface renewal/Schur-complement
argument, analogous to the n=7 twisted renewal quotient but now derived from
this general boundary formula.
