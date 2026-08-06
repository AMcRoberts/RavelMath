# Condition-F two-matrix cocycle collapse

The sheet-forgetting theorem has been strengthened from a single summed
adjacency to a two-generator statement.

For colored transport channels e with generator c(e) in {0,1}, the lifted
operators are

    A_i = sum_{c(e)=i} E_{uv} tensor P_e,

and the channel-count operators are

    B_i = sum_{c(e)=i} E_{uv}.

Because every colored base edge has one permutation lift from every initial
sheet, every word w in {0,1}* has unique path lifting.  Therefore row path
counts agree word by word for A_w and B_w.  The previous equality for
A_0+A_1 and B_0+B_1 is a corollary obtained only after preserving the pair.

Implemented operations:

- `derive_two_matrix_permutation_skew_product`
- `derive_condition_f_two_matrix_collapse`

The focused regression checks all 8,191 binary words through length 12 for a
quotient in which generator 0 carries identity holonomy and generator 1
carries swap holonomy.  It also rejects non-binary channel classifications.

The remaining project-level task is semantic rather than spectral: derive the
0/1 generator label from the actual Condition-F prefix/carry channel grammar,
instead of supplying it to the generic theorem.  Once those channel labels are
exposed, the pair collapse applies directly and the old one-matrix collapse is
only the summed corollary.
