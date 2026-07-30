# Live Python capability inventory

Status: 40 live `.py` sources as of 2026-07-28. This is a migration
map, not a deletion list. Archive a source only after its capability
has a bounded native implementation, a Lua driver where orchestration
is useful, and fixture parity or a documented correction.

## Fibonacci selection and information cost

Shared substrate:

- `fibmodel.py`

Finite/exact family and scans:

- `tsirelson_family.py`, `tsirelson_family2.py`
- `run_chsh_big.py`
- `slack_close.py`, `slack_hunt.py`
- `w1_complete.py`, `w1_fullfunction.py`

Discrete/exact algebra variants:

- `discrete_stride_fresh.py`
- `null_audit.py`
- `verify_core.py`
- `w2_exact.py`, `w2_field.py`
- `inside_view.py`
- `swap_step1.py`, `swap_step2.py`

Native coverage already available:

- `fibonacci_selection.hpp`: exact infinite-chain polarizer measure;
- `fibonacci_finite.hpp`: finite polarizer and fixed-functional Rule 30;
- Lua: `fibonacci_selection.{exact,exact_quantum}` and
  `fibonacci_finite.run`;
- packed arbitrary parity-functional correlations and retargeting:
  `fibonacci_dynamics.{correlation_matrix,retarget}`;
- bounded snapped local-window evaluation:
  `fibonacci_dynamics.local_windows`.

The packed object replaced `cost_min.py` and `cost_min2.py` without
allocating their dense `nf × N` floating-point arrays. Historical
fixture parity includes the selected functional quadruple, its four
correlators, optimized CHSH and TV, the full polarizer scan, and the
analytic mean KL value. Linear/convex optimization remains a separate
capability for the `w1`/slack scripts.

The earlier dynamical implementations are replaced by
`lua/scripts/dynamical_family.lua`. At `N=2^23` the
native run reproduces the v3 slope, four radius-8 correlators, and
retargeted CHSH at printed precision. It also preserves a correction:
the v2 fixture reports zero locality gap for every sampled radius and
therefore does not support its claimed `Theta(1/R)` law.

## Spectre geometry, adjacency, CA, and lineage

Core producers:

- `spectre_gen.py`, `spectre_gen_lineage.py`
- `spectre_ca.py`
- `spectre_hex.py`, `spectre_hex_ca.py`

Experiment drivers:

- `check_natural_correlation.py`
- `second_2root2_construction.py`
- `spectre_chsh.py`, `spectre_nomic_exclusion.py`
- `ultrametric_chsh.py`, `ultrametric_orientation.py`,
  `ultrametric_scaling.py`
- `gen_1000.py`, `gen_hex.py`
- `debug_hex.py`, `debug_hex2.py`

Render-only consumers:

- `render_1000.py`, `render_ca_gif.py`, `render_gif.py`,
  `render_hex_ca.py`

Legacy alternate geometry:

- `spectre.py`

Existing Lua transition/coordinate/lineage machinery overlaps this
family, but equivalence of tile identity, crop semantics, adjacency,
and lineage addresses has not been established. Do not retire the
Python producers merely because Lua can draw or traverse a Spectre
graph.

## Substitution ceiling remnants

- `big_shot.py`
- `ceiling_corrected.py`

These overlap the native thermometer/substitution surface but require
fixture-level comparison of their all-offset correlation ceiling
before retirement.

## Migration order

1. Extract native MI/TV primitives; preserve optimization claims as
   claims until independently checked.
2. Compare the live Lua Spectre coordinate graph against the Python
   generated/cropped tile graph using stable serialized fixtures.
3. Move renderers last; they are peripheral and may be replaced by
   data export rather than a native raster stack.
