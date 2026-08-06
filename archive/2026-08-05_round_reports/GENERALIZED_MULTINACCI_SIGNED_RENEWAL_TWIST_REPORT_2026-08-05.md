# Generalized multinacci signed renewal twist

The prefix-position deformation for

    sigma(i) = 0^m (i+1),   sigma(D-1)=0

is not a new positive transport generator.  Every concrete contact-boundary
edge carries a signed prefix defect delta.  Its exact factorization is

    delta = 0   -> Q, roof time 1, zero side voltage
    delta != 0  -> R^|delta|, roof time |delta|, side voltage sign(delta).

Thus the generalized-multinacci contact boundary is a first-return/suspension
system over the original primitive Q/R grammar.  The magnitude of the prefix
defect is a bounded renewal roof cocycle and the sign is an independent Z/2
orientation voltage.  Subdivision of each R^d macro edge into d primitive R
edges recovers a graph with exactly two primitive generators, and first-return
compression recovers the original contact-boundary adjacency exactly.

The certificate was checked on the actual source-generated boundary systems,
not a model graph, for m=1,2,3 and D=2,3,4,5.  In every nonempty case:

- every macro word is Q or R^d;
- maximum roof time is at most m;
- the sign coordinate is independent of the positive word;
- the primitive expanded alphabet is exactly {Q,R};
- first-return reconstruction equals the original G_B matrix.

This identifies the specific twist type needed for the next comparison:

    finite signed renewal twist = bounded suspension cocycle x Z/2 voltage.

The remaining uniform-D task is therefore not generator discovery.  It is to
construct the old n-bonacci comparison on the primitive Q/R suspension and
show that the renewal/voltage coordinates commute with, or are dominated by,
the same intertwiner.
