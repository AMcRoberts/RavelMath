# Continuation-controller family algebra

The earlier phrase "one controller" conflated four objects:

1. the shared radius-one transition plant;
2. terminal endpoint-face monitors;
3. residual continuation controllers;
4. the coupled first-return acceptance language.

The corrected algebra starts from every compatible endpoint monitor and closes
under the three predecessor operators `Pre_-1`, `Pre_0`, and `Pre_1`.

Exact closure:

- n=3: 26 plant states,
  26 distinct endpoint generators,
  738 residual controllers,
  2214 predecessor-table entries.

- n=4: 80 plant states,
  80 distinct endpoint generators,
  48147 residual controllers,
  144441 predecessor-table entries.

Both families replay as exactly closed under all three input digits.

The prior corpus contained only 379 observed residual classes for n=3 and 1,424
for n=4. Full generator closure is larger: 738 and 48,147 respectively. Thus
the corpus characterized a sampled subfamily, while this checkpoint constructs
the complete finite continuation-controller algebra for each fixed dimension.

Every residual controller occurring in the 2,920 exact first-return
certificates belongs to the corresponding closed family. No corpus residual is
missing.

The family should be read as an algebra of continuation policies:
- endpoint monitors are generators;
- predecessor is the family operation;
- a digit suffix denotes the composition of predecessor operations;
- membership means that some plant run reaches the terminal monitor.

The remaining universal seam is dimension uniformity. The finite algebra is
complete for fixed n, but its raw bitset representation grows with `3^n - 1`.
The next target is a symbolic presentation of this family algebra by generators,
relations, and bounded profile coordinates.
