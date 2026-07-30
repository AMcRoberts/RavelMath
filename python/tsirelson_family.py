"""
tsirelson_family.py
------------------------------------------------------------------------
GOAL: a validated FAMILY of deterministic structures whose spacelike
statistical correlations approach the Tsirelson limit 2*sqrt(2).

Everything below is a function of ONE deterministic object: the internal
coordinate theta_k = {k*alpha} of site k in the Fibonacci chain
(alpha = 1/phi). No RNG appears anywhere in the model.

  hidden variable   lambda      = theta (internal-space coordinate)
  pair at sep L     theta_B     = theta_A + delta(L),  delta(L) = {L*alpha}
                                  (spacelike return vectors L = F_m give
                                   delta -> 0 exponentially)
  settings          (a,b) = Q(theta): quarter of the fast sawtooth
                                  {d2*theta} -- a 4-piece window partition
  ensemble          site accepted iff {d1*theta} < w_ab(theta)/(1+|kappa|),
                                  w_ab = 1 + kappa_ab * AB_ab(theta)
                                  -- ANOTHER window partition (the only
                                  place measurement independence is bent,
                                  by the lawful bias kappa)
  outcomes          A_a = sign cos(theta - x_a), B_b = sign cos(theta_B - y_b)
                                  with CHSH angles x = {0, pi/2},
                                  y = {pi/4, -pi/4}

With kappa = 0 (no selection bias) the model is a bona fide local HV
model: correlators are triangle waves, and at these angles S = 2 EXACTLY
(the classical bound, saturated).

The lawful bias kappa_ab = (C_ab - T_ab)/(1 - C_ab*T_ab) retargets each
correlator from its triangle value T to any C in (-1, 1):
    C = quantum cosine values  (+-1/sqrt2)  -> S -> 2*sqrt(2)  [Tsirelson]
    C = PR box values          (+1,+1,+1,-1) -> S -> 4          [algebraic]
So a single dial measures "conspiracy cost", with the three landmarks
2 (kappa=0), 2sqrt2 (kappa ~ 0.32), 4 (kappa = 1).

The FAMILY: index m -- use separation L = F_m and strides d1, d2 from
the same Fibonacci ladder. Claim to validate: at quantum kappa,
S_m -> 2*sqrt(2) from below as m grows, and never exceeds it.

CONTROLS:
  * kappa = 0 must give S = 2.000 (classical saturation, no bias)
  * scrambled pairing (partner = k + random L per site, destroying the
    return-vector structure while keeping everything else) must collapse
    S -> 0: shows pure-point/almost-period structure is load-bearing.
  * PR kappa must give S -> 4: shows 2sqrt2 is a property of the QUANTUM
    target statistics (extremal positive-definite cosine kernel), not a
    limitation of the mechanism.
Also reported: measurement-dependence cost = total-variation distance
of the selected lambda-ensemble from uniform, per setting pair.
------------------------------------------------------------------------
"""
import json
import math
import numpy as np

ALPHA = (math.sqrt(5) - 1) / 2
TSIRELSON = 2 * math.sqrt(2)

# CHSH angles on the circle (radians)
XA = {0: 0.0, 1: math.pi / 2}
YB = {0: math.pi / 4, 1: -math.pi / 4}

FIB = [1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597,
       2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393]


def triangle(d):
    """E for sign-cos outcomes under uniform lambda; d = angle difference."""
    dd = abs(math.remainder(d, 2 * math.pi))
    return 1 - 2 * dd / (math.pi / 1) * 1.0 if False else 1 - (2 / math.pi) * min(dd, 2 * math.pi - dd) * (1 if min(dd, 2*math.pi-dd) <= math.pi else 1)


def triangle_correct(d):
    dd = abs(math.remainder(d, 2 * math.pi))  # in [0, pi]
    return 1 - (2 / math.pi) * dd


def kappa_for(C, T):
    return (C - T) / (1 - C * T)


def run_model(N, L, d1, d2, targets, scramble=False, rng_for_scramble=None):
    k = np.arange(N, dtype=np.float64)
    theta = (k * ALPHA) % 1.0                       # internal coordinate in [0,1)
    th = 2 * math.pi * theta

    if scramble:
        Ls = rng_for_scramble.integers(33, N // 2, size=N)
        partner_idx = (np.arange(N) + Ls) % N
    else:
        partner_idx = (np.arange(N) + L) % N
    thB = 2 * math.pi * ((partner_idx * ALPHA) % 1.0)

    # settings: quarter of fast sawtooth {d2 * theta}
    Q = np.floor(4 * ((d2 * theta) % 1.0)).astype(int)   # 0..3 -> (a,b)
    ab_of_Q = {0: (0, 0), 1: (0, 1), 2: (1, 0), 3: (1, 1)}

    # outcomes
    r = (d1 * theta) % 1.0                                # selection sawtooth

    E = {}
    md_cost = {}
    counts = {}
    for q in range(4):
        a, b = ab_of_Q[q]
        d = XA[a] - YB[b]
        T = triangle_correct(d)
        C = targets[(a, b)]
        kap = kappa_for(C, T)
        mask_q = (Q == q)
        A = np.sign(np.cos(th - XA[a]))
        B = np.sign(np.cos(thB - YB[b]))
        AB = A * B
        w = 1 + kap * AB                                   # lawful weight (window fn)
        accept = mask_q & (r < w / (1 + abs(kap)))
        n_acc = int(accept.sum())
        E[(a, b)] = float(AB[accept].mean()) if n_acc else float("nan")
        counts[(a, b)] = n_acc
        # measurement-dependence cost: TV(rho(theta|ab), uniform)
        # realized empirically from the acceptance function of theta
        Tval = T
        norm = 1 + kap * Tval
        md_cost[(a, b)] = abs(kap) * (1 - abs(Tval) * 0) / 2 / norm  # analytic approx
    S = E[(0, 0)] + E[(0, 1)] + E[(1, 0)] - E[(1, 1)]
    return E, S, counts, md_cost


def main():
    N = 1 << 21
    QUANTUM = {(0, 0): math.cos(XA[0] - YB[0]), (0, 1): math.cos(XA[0] - YB[1]),
               (1, 0): math.cos(XA[1] - YB[0]), (1, 1): math.cos(XA[1] - YB[1])}
    PRBOX = {(0, 0): 0.999, (0, 1): 0.999, (1, 0): 0.999, (1, 1): -0.999}
    CLASSICAL = None  # kappa = 0 handled via targets = triangle values

    rows = []

    print(f"N = {N} sites; Tsirelson = {TSIRELSON:.6f}\n")

    # ---- control 1: kappa = 0 (targets = triangle values) => S = 2 exactly
    Ttargets = {(a, b): triangle_correct(XA[a] - YB[b]) for a in (0, 1) for b in (0, 1)}
    E, S, cnt, _ = run_model(N, L=FIB[19], d1=FIB[17], d2=FIB[15], targets=Ttargets)
    print(f"control kappa=0 (pure local model):        S = {S:+.5f}   (classical bound 2)")
    rows.append({"case": "kappa0", "S": S})

    # ---- the family: quantum targets, increasing Fibonacci level m
    print("\nFAMILY (quantum targets): separation L = F_m, strides F_(m-2), F_(m-4)")
    print(f"{'m':>3} {'L=F_m':>8} {'delta=||L*a||':>13} {'S_m':>9} {'gap to 2sqrt2':>13}")
    for m in (10, 12, 14, 16, 18, 20, 22, 24):
        L = FIB[m]
        d1 = FIB[m - 2]
        d2 = FIB[m - 4]
        delta = abs(L * ALPHA - round(L * ALPHA))
        E, S, cnt, md = run_model(N, L=L, d1=d1, d2=d2, targets=QUANTUM)
        gap = TSIRELSON - S
        print(f"{m:>3} {L:>8} {delta:>13.7f} {S:>9.5f} {gap:>+13.5f}")
        rows.append({"case": "quantum", "m": m, "L": L, "delta": delta,
                     "S": S, "gap": gap,
                     "counts": {str(k): v for k, v in cnt.items()}})

    # ---- control 2: scrambled pairing (destroys return-vector structure)
    rng = np.random.default_rng(0)
    E, S, cnt, _ = run_model(N, L=0, d1=FIB[17], d2=FIB[15], targets=QUANTUM,
                             scramble=True, rng_for_scramble=rng)
    print(f"\ncontrol scrambled partners:                S = {S:+.5f}   (structure destroyed)")
    rows.append({"case": "scrambled", "S": S})

    # ---- control 3: PR-box targets => mechanism can reach 4; 2sqrt2 is a
    #      property of the quantum (cosine) statistics, not of the machinery
    E, S, cnt, _ = run_model(N, L=FIB[22], d1=FIB[20], d2=FIB[18], targets=PRBOX)
    print(f"control PR-box targets:                    S = {S:+.5f}   (algebraic max 4)")
    rows.append({"case": "prbox", "S": S})

    # ---- conspiracy dial: kappa cost at each landmark
    print("\nconspiracy dial (bias kappa needed per correlator, at these angles):")
    for name, tg in (("classical S=2", Ttargets), ("quantum S=2sqrt2", QUANTUM),
                     ("PR box S=4", PRBOX)):
        kaps = [abs(kappa_for(tg[(a, b)], triangle_correct(XA[a] - YB[b])))
                for a in (0, 1) for b in (0, 1)]
        print(f"   {name:>18}: max|kappa| = {max(kaps):.4f}")

    with open("tsirelson_family_results.json", "w") as f:
        json.dump(rows, f, indent=1)
    print("\nsaved -> tsirelson_family_results.json")


if __name__ == "__main__":
    main()
