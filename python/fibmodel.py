"""
fibmodel.py -- shared machinery for the Tsirelson-family analyses.

Model recap (all deterministic, no RNG):
  theta_k = {k*alpha}, alpha = 1/phi          (internal coordinate)
  settings (a,b) = quarter of {d2*theta}      (window partition, d2 = F_{p-1})
  selection: accept iff {d1*theta} < w/(1+|kappa|), w = 1 + kappa_ab*AB
                                              (window partition, d1 = F_p)
  pairs (k, k+L), L = F_m                     (return vectors, delta_m -> 0)
  outcomes:
    'polarizer' mode: A_a = sgn cos(2pi theta - x_a), B_b likewise with y_b
    'ca' mode:        run a radius-1 CA (rule 30) for T_steps on the letter
                      sequence; outcomes are fixed XOR functionals of the
                      frozen output, different functionals per side/setting.
                      Light cone radius = T_steps exactly.
"""
import math
import numpy as np

ALPHA = (math.sqrt(5) - 1) / 2
TS = 2 * math.sqrt(2)
XA = {0: 0.0, 1: math.pi / 2}
YB = {0: math.pi / 4, 1: -math.pi / 4}
FIB = [1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597,
       2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393,
       196418, 317811, 514229]


def tri(d):
    dd = abs(math.remainder(d, 2 * math.pi))
    return 1 - (2 / math.pi) * dd


QUANTUM = {(a, b): math.cos(XA[a] - YB[b]) for a in (0, 1) for b in (0, 1)}
TRIANGLE = {(a, b): tri(XA[a] - YB[b]) for a in (0, 1) for b in (0, 1)}


def cosine_family(sigma):
    """One-parameter target family C_ab = +-sigma/4: sigma=2 classical
    (identical to TRIANGLE at these angles), 2sqrt2 quantum, 4 PR box."""
    s = sigma / 4
    return {(0, 0): s, (0, 1): s, (1, 0): s, (1, 1): -s}


def letters(N):
    k = np.arange(N, dtype=np.int64)
    return (((k * ALPHA) % 1.0) < ALPHA).astype(np.uint8)


def evolve_rule30(s, T):
    for _ in range(T):
        l = np.roll(s, 1)
        r = np.roll(s, -1)
        s = l ^ (s | r)
    return s


def ca_outcomes(N, T_steps):
    """Two fixed +-1 functionals of the frozen CA output per side.
    A-side: XOR at offsets (0,1) / (0,3); B-side: XOR at offsets (0,2)/(0,5).
    All are radius <= T_steps + 5 local functions of the letters."""
    s = evolve_rule30(letters(N), T_steps)
    f = lambda off: 1.0 - 2.0 * (s ^ np.roll(s, -off))
    return {("A", 0): f(1), ("A", 1): f(3), ("B", 0): f(2), ("B", 1): f(5)}


def run(N, L, d1, d2, targets, mode="polarizer", T_steps=16, base_T=None,
        return_extras=False):
    """One model evaluation. `targets` maps (a,b) -> desired correlator C.
    `base_T` maps (a,b) -> unbiased correlator T (defaults: analytic triangle
    for polarizer mode; MUST be supplied, measured at kappa=0, for ca mode).
    Returns (E, S, extras)."""
    idx = np.arange(N, dtype=np.int64)
    theta = (idx * ALPHA) % 1.0
    Q = np.floor(4 * ((d2 * theta) % 1.0)).astype(np.int8)
    r = (d1 * theta) % 1.0

    if mode == "polarizer":
        th = 2 * math.pi * theta
        thB = 2 * math.pi * (((idx + L) * ALPHA) % 1.0)
        Afun = {a: np.sign(np.cos(th - XA[a])) for a in (0, 1)}
        Bfun = {b: np.sign(np.cos(thB - YB[b])) for b in (0, 1)}
        if base_T is None:
            base_T = TRIANGLE
    else:
        oc = ca_outcomes(N, T_steps)
        Afun = {a: oc[("A", a)] for a in (0, 1)}
        Bfun = {b: np.roll(oc[("B", b)], -L) for b in (0, 1)}
        if base_T is None and targets is not None:
            raise ValueError("ca mode retargeting requires measured base_T")

    E, tv, kl, cnt, kaps = {}, {}, {}, {}, {}
    for q, (a, b) in enumerate([(0, 0), (0, 1), (1, 0), (1, 1)]):
        AB = Afun[a] * Bfun[b]
        mask = (Q == q)
        if targets is None:                      # kappa = 0 (unbiased)
            acc = mask
            kap = 0.0
        else:
            T0 = base_T[(a, b)]
            C = targets[(a, b)]
            kap = (C - T0) / (1 - C * T0)
            acc = mask & (r < (1 + kap * AB) / (1 + abs(kap)))
        kaps[(a, b)] = kap
        n = int(acc.sum())
        cnt[(a, b)] = n
        E[(a, b)] = float(AB[acc].mean()) if n else float("nan")
        if return_extras:
            h_acc, _ = np.histogram(theta[acc], bins=1024, range=(0, 1),
                                    density=True)
            tv[(a, b)] = 0.5 * float(np.abs(h_acc - 1.0).mean())
            rho = np.clip(h_acc, 1e-12, None)
            kl[(a, b)] = float((h_acc * np.log(rho)).mean())  # KL vs uniform, nats
    S = E[(0, 0)] + E[(0, 1)] + E[(1, 0)] - E[(1, 1)]
    extras = {"tv": tv, "kl": kl, "counts": cnt, "kappa": kaps}
    return (E, S, extras) if return_extras else (E, S)
