# Closed-Form Label Recurrence and Transient Rank

## Result

The bounded alternating-zero symbolic graph no longer uses Tarjan SCC decomposition to discover its recurrent schedules.

For every odd dimension `D >= 5`, the engine constructs exactly two predicted schedules, one for each orientation.

For orientation `-1`:

- phase `0`: `(0,0)` and `(1,2)`
- phase `1`: `(1,0)` and `(2,0)`
- phases `z >= 2`: `(0,1)` for even `z`, `(1,0)` for odd `z`

For orientation `+1`:

- phase `0`: `(0,0)` and `(2,1)`
- phase `1`: `(0,1)` and `(0,2)`
- phases `z >= 2`: `(1,0)` for even `z`, `(0,1)` for odd `z`

Each schedule therefore contains `D+2` concrete states. Canonical exact prefix replay proves the internal renewal transition, the phase twist `z -> z-1 mod D`, and the unique doubled phase `z=2`.

The predicted schedules need not be terminal in the full symbolic graph: they may emit edges into transient states. The certificate therefore requires a complete internal renewal subgraph, not ambient sink closure.

## Transient exclusion

After removing the predicted schedule states, the engine applies reverse Kahn elimination to the remaining exact prefix graph. The resulting rank satisfies

`u -> v` among non-schedule states implies `rank(u) > rank(v)`.

Thus the remainder contains no directed cycle and hence no unclassified recurrent SCC. This is a replayable well-founded certificate, not an SCC search.

Observed checks:

| Dimension | Predicted schedules | Transient states | Maximum rank |
|---|---:|---:|---:|
| 5 | 2 | 236 | 2 |
| 6 | 0 | 240 | 2 |
| 7 | 2 | 668 | 2 |
| 8 | 0 | 560 | 2 |
| 9 | 2 | 1436 | 2 |

Even dimensions have no alternating-zero renewal schedule; the entire bounded symbolic family is transient.

## Engineering changes

`NBonacciSymbolicLabelScheduleProof` now records:

- `closed_form_schedules_derived`
- `predicted_schedules_closed`
- `transient_label_rank_derived`
- `scc_enumeration_avoided`
- transient state count and maximum rank

`derive_nbonacci_symbolic_label_schedule` now:

1. builds the bounded exact symbolic prefix graph;
2. materializes the two schedules from the formulas above;
3. replay-checks their internal renewal grammar and prefix multiplicities;
4. removes those states;
5. derives a strict topological rank for every remaining state.

## Remaining universal seam

The rank is presently computed by elimination on the bounded label graph. The next strengthening is to derive the rank value directly from endpoint-label formulas, eliminating even bounded graph traversal. The observed maximum rank `2` suggests a short closed-form classification into:

1. schedule states;
2. states mapping directly into a schedule or dead end;
3. states mapping into class 2.
