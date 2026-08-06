# Graph structural attractor and seam

The graph campaign treats certificates as evidence, not endpoints.  It computes
an exact SCC partition, condensation DAG, unique-sink attractor basin, and
adjacency matrix.  The resulting structure is compared against installed
machinery models.

The first installed model is an n-cycle recurrent core with one transient
feeder vertex.  Its certificate closes:

- two SCCs;
- one condensation edge;
- one sink/recurrent SCC;
- full basin attraction to that SCC;
- exact graph-to-adjacency-matrix transport;
- total directed edge count n+1.

The model then attracts proof search to the actual seam rather than stopping at
the certificate.  The open seam is:

1. canonically identify the recurrent SCC with the cycle family C_n;
2. transport that presentation to the characteristic polynomial of its
   adjacency/permutation matrix;
3. separate the transient factor from the recurrent spectral factor;
4. generalize the discovered feeder/core decomposition beyond this seed family.

This mirrors universal-n: the certificate reveals a structural decomposition,
and that decomposition determines the next reusable derivation operations.
