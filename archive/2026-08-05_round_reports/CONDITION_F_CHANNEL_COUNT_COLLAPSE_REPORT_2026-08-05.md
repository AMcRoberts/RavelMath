# Condition-F Channel-Count Collapse

The twisted lift has adjacency

\[
A=\sum_{e:u\to v} E_{uv}\otimes P_e,
\]

where each channel matrix `P_e` is a permutation.  Forgetting the sheet
permutations leaves the directed multigraph adjacency `B`, with one parallel
base edge for each channel.

Every base path has exactly one lift from each chosen point of the source
fibre.  Therefore, for every length `k`, every concrete state over role `u`
has exactly the same number of length-`k` continuations as `u` in the base
multigraph.  Hence the row path-growth sequences agree at every length and

\[
\rho(A)=\rho(B).
\]

Holonomy may glue or split lifted SCCs, alter periods, and mix grades, but it
cannot alter exponential path growth.  Spectral maximality is therefore proved
on the channel-count quotient, not by classifying holonomy-resolved lifted
components.

Implemented in:

- `include/ravel/proof/condition_f_channel_count_collapse.hpp`
- `tests/condition_f_channel_count_collapse_test.cpp`

The universal-dominance composer now accepts the collapse result directly.
The older terminal/pure-grade/mixed-grade component routing remains useful for
structural interpretation and diagnostics, but is not part of the spectral
proof dependency.
