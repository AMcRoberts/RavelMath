"""
beta_expansion_thread_a4.py -- Renyi/Parry beta-expansion d_beta(1)
for Thread A4 (docs/DIRECTION_AND_OPEN_THREADS.md), computed exactly
via characteristic polynomials + high-precision root-finding.

Origin: Thread A4 originally proposed "the Pisot continued fraction
of 1/beta_{0,1}... periodic for all Pisot numbers, period length
distinguishes Class I/II/III" (docs/DIRECTION_AND_OPEN_THREADS.md).
That's wrong as literally stated -- by Lagrange's theorem a classical
continued fraction is eventually periodic iff the number is a
quadratic irrational, and beta_{0,1} (root of x^3-x-1) is cubic. The
real nearby theorem (Bertrand 1977 / Schmidt 1980) is about the
Renyi/Parry beta-expansion d_beta(1), a digit expansion, not a
fraction -- this script computes that instead. See
docs/FINDINGS_FOR_CITATION.md Finding 6.5 for the write-up.

Two real bugs found and fixed while building this (both from naive
floating-point evaluation near an ALGEBRAIC boundary, where the
greedy step lands exactly on an integer):
  1. First attempt (200-dps mpmath matrix eigenvalues, 150 digits)
     mistook sigma_{0,1}'s genuinely finite expansion for a period-5
     one -- accumulated rounding pushed an exact-zero residue to a
     tiny positive value that resurfaced periodically after repeated
     multiplication by beta.
  2. Second attempt (500-dps, exact sympy charpolys, but a fixed bad
     initial guess for mpmath.findroot) converged to the WRONG root
     (even a negative one) for sigma_{3,1}/sigma_{4,1} -- fixed by
     using mpmath.polyroots to get ALL roots and selecting the real
     dominant one, rather than trusting a single Newton guess.
The current version uses exact sympy characteristic polynomials,
mpmath.polyroots at 3000-digit precision, and near-integer snapping
(near_int_round) to correctly resolve steps that land exactly on an
integer algebraically but evaluate a hair below it in floating point.

Finding: termination (i.e. eventual period = 1, all zeros) does NOT
distinguish Class I/II/III -- every tested Class I, most Class II,
and the one tested Class III (unimodular) example all terminate. The
one outlier (sigma_{0,2}) is also the only non-unimodular candidate
tested, so the anomaly may track unimodularity, not class.
"""
import sympy as sp
import mpmath as mp

mp.mp.dps = 3000

x = sp.symbols('x')


def companion_charpoly(M):
    return sp.expand(sp.Matrix(M).charpoly(x).as_expr())


def pisot_root(charpoly_expr):
    """Real dominant root of an integer characteristic polynomial,
    found by computing ALL roots (not a single Newton guess, which
    can converge to the wrong -- even negative -- root) and picking
    the largest real one."""
    poly = sp.Poly(charpoly_expr, x)
    coeffs = [mp.mpf(int(c)) for c in poly.all_coeffs()]
    roots = mp.polyroots(coeffs, maxsteps=400, extraprec=2500)
    real_roots = [mp.re(r) for r in roots if abs(mp.im(r)) < mp.mpf('1e-500')]
    return max(real_roots) if real_roots else None


NEAR_INT_TOL = mp.mpf(10) ** (-2500)


def near_int_round(v):
    nearest = mp.nint(v)
    return nearest if abs(v - nearest) < NEAR_INT_TOL else mp.floor(v)


def beta_expansion(beta, n_digits):
    """Greedy Renyi/Parry expansion d_beta(1) = (a_1, a_2, ...)."""
    a1 = near_int_round(beta)
    r = beta - a1
    digits = [int(a1)]
    terminated_at = None
    for i in range(n_digits - 1):
        if abs(r) < NEAR_INT_TOL:
            if terminated_at is None:
                terminated_at = i + 1
            digits.append(0)
            r = mp.mpf(0)
            continue
        prod = beta * r
        a = near_int_round(prod)
        r = prod - a
        digits.append(int(a))
    return digits, terminated_at


def find_period(digits, min_tail=10):
    n = len(digits)
    for period in range(1, n // 3):
        for preperiod in range(0, n - 2 * period - min_tail):
            tail = digits[preperiod:]
            if all(tail[i] == tail[i + period] for i in range(len(tail) - period)):
                return preperiod, period
    return None, None


def sigma_ab_matrix(a, b):
    return [[a, a, 1], [b, 0, 0], [1, 1, 0]]


CANDIDATES = {
    "Tribonacci (I, n=3)":  [[1, 1, 1], [1, 0, 0], [0, 1, 0]],
    "Tetrabonacci (I,n=4)": [[1, 1, 1, 1], [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0]],
    "Pentanacci (I,n=5)":   [[1, 1, 1, 1, 1], [1, 0, 0, 0, 0], [0, 1, 0, 0, 0],
                              [0, 0, 1, 0, 0], [0, 0, 0, 1, 0]],
    "sigma_{0,1} (III)":    sigma_ab_matrix(0, 1),
    "sigma_{1,1} (II)":     sigma_ab_matrix(1, 1),
    "sigma_{2,1} (II)":     sigma_ab_matrix(2, 1),
    "sigma_{3,1} (II)":     sigma_ab_matrix(3, 1),
    "sigma_{4,1} (II)":     sigma_ab_matrix(4, 1),
    "sigma_{5,1} (II)":     sigma_ab_matrix(5, 1),
    "sigma_1 (II, 3L)":     [[2, 1, 1], [1, 0, 0], [0, 1, 0]],
    "sigma_2 (II, 3L)":     [[2, 2, 1], [1, 0, 0], [0, 1, 0]],
    # CORRECTION (Finding 32, docs/FINDINGS_FOR_CITATION.md): this
    # matrix is NOT actually Pisot -- pisot_classify_3x3 certifies
    # its complex-conjugate pair at modulus ~1.1466 > 1. Schmidt's
    # eventual-periodicity theorem does not apply, and none of the
    # "unresolved" results below for this entry mean anything more
    # than that. Kept in the candidate list (not deleted) as a record
    # of the mistake, per this project's practice of not erasing
    # wrong turns -- but do not trust its classification label.
    "sigma_{0,2} (III, NOT ACTUALLY PISOT -- see Finding 32)": sigma_ab_matrix(0, 2),
}


def main():
    results = []
    for name, M in CANDIDATES.items():
        cp = companion_charpoly(M)
        beta = pisot_root(cp)
        n_digits = 400 if "0,2" in name else 60
        digits, term = beta_expansion(beta, n_digits)
        preperiod, period = find_period(digits, min_tail=15)
        kind = (f"finite(len={term})" if term else
                (f"periodic(pp={preperiod},per={period})" if period else "UNRESOLVED"))
        print(f"{name}: beta={mp.nstr(beta, 20)}  d_beta(1)={digits[:20]}  -> {kind}")
        results.append((name, kind))
    print("\n=== SUMMARY ===")
    for name, kind in results:
        print(f"  {name:28s} {kind}")


if __name__ == "__main__":
    main()
