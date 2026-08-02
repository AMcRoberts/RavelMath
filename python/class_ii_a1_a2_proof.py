"""
class_ii_a1_a2_proof.py -- symbolic proof that Item A's Thread A3
question (does sigma_{a,1}'s dominant-core admit the n-bonacci-style
involution + x^k nilpotent cofactor?) resolves NO, for every integer
a>=2 at once, not just at the numerically tested values.

See docs/FINDINGS_FOR_CITATION.md Finding 10 for the write-up.

Built from two already-proven, a-independent pieces:
  - Theorem 6's explicit eleven-state contact-core matrix T_a for
    sigma_{a,1} (docs/CLASS_II_CONTACT_MATRIX.md), transcribed here
    and independently re-verified against its own published
    characteristic polynomial x^6[x^5-a^2x^3-a(a+1)].
  - Finding 7's proven balanced-pair-core characteristic polynomial
    x[x^3-(a+1)x^2+ax-1] (docs/CLASS_II_BP_FAMILY_THEOREM.md).

T_a's eleven (i,x,j) state labels are the SAME fixed list for every
a -- only the edge weights (a, a-1) vary -- so the involution-pairing
question is a single finite check good for all a, not a per-a probe:

  A2 (involution): pairing each state (i,x,j) with its mirror
  (j,-x,i) inside the eleven-state list finds exactly three matched
  pairs and five states whose mirror is absent from the set entirely
  (three need j=2, which no state in the list has). 6/11 matched,
  for every a -- matching Finding 5's numerically recorded "6/11"
  ratio, now shown to be a-independent by direct argument.

  A1 (nilpotent cofactor): the quotient matrix Q_sym built from the
  three matched pairs (following gb_bp_matrix_equality.cpp's exact
  orbit/quotient construction) has characteristic polynomial
  x^3 - a*x^2, degree 3. The BP-core's charpoly (Finding 7) has
  degree 4. A degree-4 polynomial cannot divide a degree-3
  polynomial (other than the zero polynomial) -- the A1 divisibility
  fails by pure degree-counting, for every a>=2 simultaneously.

Caveat this does NOT close: whether T_a's eleven states really are
the FULL dominant recurrent core for every a (not just the checked
1<=a<=8) is Finding 9/Theorem 6's own still-open premise. This proof
is conditional on that premise, same as Finding 9 itself.
"""
import sympy as sp

a, x = sp.symbols('a x')


def build_T():
    """Theorem 6's displayed 11-state matrix T_a, transcribed from
    docs/CLASS_II_CONTACT_MATRIX.md. T[i][j] = weight of the edge
    FROM state i TO state j."""
    n = 11
    T = sp.zeros(n, n)
    T[0, 1] = 1
    T[1, 10] = 1
    T[2, 10] = 1
    T[3, 2] = 1
    T[4, 5] = 1
    T[4, 7] = 1
    T[5, 4] = 1
    T[5, 6] = 1
    T[6, 3] = a
    T[6, 5] = a - 1
    T[6, 7] = a - 1
    T[7, 1] = a
    T[7, 4] = a - 1
    T[7, 6] = a - 1
    T[8, 0] = 1
    T[8, 3] = a - 1
    T[8, 5] = a
    T[8, 7] = a
    T[9, 8] = 1
    T[10, 9] = 1
    return T


# States (i, x=(x0,x1,x2), j), doc's lexicographic order.
STATES = [
    (2, (-1, 1, 0), 1),   # 0
    (1, (1, -1, -1), 0),  # 1
    (2, (1, 0, -1), 0),   # 2
    (0, (-1, 1, 1), 1),   # 3
    (1, (1, -1, 0), 0),   # 4
    (0, (-1, 1, 0), 1),   # 5
    (0, (1, -1, 0), 0),   # 6
    (0, (-1, 1, 0), 0),   # 7
    (0, (0, -1, 0), 0),   # 8
    (2, (0, 1, -1), 0),   # 9
    (0, (-1, 0, 1), 1),   # 10
]


def mirror(s):
    i, xv, j = s
    return (j, tuple(-c for c in xv), i)


def find_pairs(states):
    index_of = {s: k for k, s in enumerate(states)}
    pairs, unpaired, seen = [], [], set()
    for k, s in enumerate(states):
        if k in seen:
            continue
        m = mirror(s)
        if m in index_of and index_of[m] != k:
            w = index_of[m]
            pairs.append((k, w))
            seen.add(k)
            seen.add(w)
        else:
            unpaired.append(k)
    return pairs, unpaired


def build_qsym(T, pairs, n):
    N = len(pairs)
    rep = [p[0] for p in pairs]
    orbit_of = {}
    for oid, (u, w) in enumerate(pairs):
        orbit_of[u] = oid
        orbit_of[w] = oid
    Qsym = sp.zeros(N, N)
    for oa in range(N):
        ra = rep[oa]
        for dest in range(n):
            w_edge = T[ra, dest]
            if w_edge == 0 or dest not in orbit_of:
                continue
            b = orbit_of[dest]
            Qsym[oa, b] += w_edge
    return Qsym


def main():
    T = build_T()
    n = T.shape[0]
    charpoly_T = sp.expand((x * sp.eye(n) - T).det())
    claimed = sp.expand(x**6 * (x**5 - a**2 * x**3 - a * (a + 1)))
    print("charpoly(T_a) computed:", charpoly_T)
    print("claimed (Theorem 6):   ", claimed)
    print("MATCH:", sp.simplify(charpoly_T - claimed) == 0)
    print()

    pairs, unpaired = find_pairs(STATES)
    print("Mirror pairs (rep, partner):", pairs)
    print("Unpaired states:", unpaired)
    print(f"A2 result: {2*len(pairs)} / {n} matched, for every a")
    print()

    Qsym = build_qsym(T, pairs, n)
    print("Q_sym =")
    sp.pprint(Qsym)
    cp_qsym = sp.expand((x * sp.eye(len(pairs)) - Qsym).det())
    print("charpoly(Q_sym) =", cp_qsym, " factored:", sp.factor(cp_qsym))
    print()

    bp_charpoly = sp.expand(x * (x**3 - (a + 1) * x**2 + a * x - 1))
    print("BP charpoly (Finding 7):", bp_charpoly,
          " degree =", sp.degree(bp_charpoly, x))
    print("Q_sym_GB charpoly degree =", sp.degree(cp_qsym, x))
    print()
    print("A1 result: degree(Q_sym_GB) < degree(BP) => BP cannot divide")
    print("Q_sym_GB for any a -- the nilpotent-cofactor structure fails")
    print("by pure degree-counting, for every integer a>=2 at once.")


if __name__ == "__main__":
    main()
