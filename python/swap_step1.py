"""
swap_step1.py -- first concrete pass of the perception swap.

(a) SPECTRAL CORRELATOR IDENTITY: E_ab(delta) recomputed as a sum over
    the dual module (Fourier-Bohr characters) vs direct location-space
    average. The swapped object: amplitudes A_hat(n), B_hat(n) --
    "symmetries vectoring toward outcomes".
(b) A2 CLOSED FORM from the swapped view: the tilt correlator is
    E = (T+kappa)/(1+kappa T) with T(delta) the character sum; chain
    rule at the CHSH point gives
       A2 = 8 * sum_ctx sign * d2E/dT2 * (dT/ddelta)^2/16... =>
       A2 = -64 * kappa * (1-kappa^2) / (1 + kappa/2)^3   (T0 = 1/2)
    Compare the measured -11.86 from theorem_v3 (corrected: original -11.84
    figure averaged across a regime crossover, caught by independent
    clean-room reimplementation).
    [historical: buggy pooled fit gave -11.84/-12.07 depending on range]
(c) GAP-LABEL MEMBERSHIP (K-theory side of W2):
    silver chain gap-label module (1D thm: Z-module of word occurrence
    probabilities) contains Z + Z*beta. rho_Q = 9-6sqrt2 = 3 - 6*beta
    -> a GAP LABEL of the silver chain, manifestly. Golden module
    Z + Z*alpha subset Q(sqrt5) cannot contain it (field-disjoint).
(d) PENTAGONAL EXTENSION (the "golden direction"): quantum correlators
    at pentagonal angles: cos(pi/5) = phi/2, cos(2pi/5) = (phi-1)/2.
    Native golden CHSH value S_pent = 3cos(pi/5) - cos(3pi/5)
    = (4phi-1)/2. Budget rho_pent at (T = 3/5, C = phi/2)
    = 4(2-phi)/(2+phi) = 4 - (8/5)sqrt5. Check module membership.
"""
import math, json
import numpy as np

SQ2, SQ5 = math.sqrt(2), math.sqrt(5)
PHI = (1+SQ5)/2
ALPHA = PHI - 1
BETA = SQ2 - 1
TS = 2*SQ2
rows = {}

# ---------- (a) spectral correlator identity ----------
G = 1 << 14
th = np.arange(G)/G
def win(x): return np.sign(np.cos(2*math.pi*th - x))
XA = {0:0.0, 1:math.pi/2}; YB = {0:math.pi/4, 1:-math.pi/4}
deltas = [0.0, 0.013, 0.037]
maxerr = 0.0
for a in (0,1):
    for b in (0,1):
        A, B = win(XA[a]), win(YB[b])
        Ah, Bh = np.fft.fft(A)/G, np.fft.fft(B)/G
        for d in deltas:
            # location side: shift B by delta (circular, exact via char sum)
            phase = np.exp(2j*math.pi*np.fft.fftfreq(G, 1/G)*d)
            E_spec = float(np.real(np.sum(Ah*np.conj(Bh)*np.conj(phase))))
            Bshift = np.sign(np.cos(2*math.pi*(th+d) - YB[b]))
            E_loc = float(np.mean(A*Bshift))
            maxerr = max(maxerr, abs(E_spec-E_loc))
print(f"(a) spectral-vs-location correlator: max |diff| = {maxerr:.2e} "
      f"(grid-edge tolerance ~ 1/G = {1/G:.1e})")
rows["spectral_identity_maxerr"] = maxerr

# ---------- (b) A2 closed form ----------
C0, T0 = 1/SQ2, 0.5
kap = (C0-T0)/(1-C0*T0)
E_TT = -2*kap*(1-kap**2)/(1+kap*T0)**3          # same |value| all 4 contexts
A2_closed = 8*(3*E_TT + E_TT)                    # signs: 3 ctx +(neg), ctx11 -(pos)
A2_closed = 8*4*E_TT
print(f"(b) A2 closed form = -64*kappa*(1-kappa^2)/(1+kappa/2)^3 "
      f"= {A2_closed:.4f}   (measured in theorem_v3, corrected: -11.86)")
rows["A2_closed"] = A2_closed

# ---------- (c) gap-label membership ----------
rho_Q = 9 - 6*SQ2
print(f"(c) rho_Q = 9-6sqrt2 = 3 - 6*beta = {3-6*BETA:.12f} "
      f"(check {rho_Q:.12f}) -> in Z + Z*beta: TRUE (coeffs 3, -6)")
print(f"    golden module Z+Z*alpha subset Q(sqrt5); rho_Q in Q(sqrt2)\\Q "
      f"-> NOT a golden-chain gap label (field-disjoint, proven)")
rows["rhoQ_silver_gap_label"] = True

# ---------- (d) pentagonal / golden-native numbers ----------
c36, c72 = math.cos(math.pi/5), math.cos(2*math.pi/5)
print(f"(d) cos36 = {c36:.6f} = phi/2 = {PHI/2:.6f}; "
      f"cos72 = {c72:.6f} = (phi-1)/2 = {(PHI-1)/2:.6f}")
S_pent = 3*c36 - math.cos(3*math.pi/5)
print(f"    native golden CHSH: S_pent = 3cos36 - cos108 = {S_pent:.6f} "
      f"= (4phi-1)/2 = {(4*PHI-1)/2:.6f}  [vs 2sqrt2 = {TS:.6f}]")
T_pent = 1 - (2/math.pi)*(math.pi/5)             # 3/5 exactly, rational
rho_pent = (1-c36)*(1+T_pent)/((1+c36)*(1-T_pent))
closed = 4 - (8/5)*SQ5
print(f"    budget at pentagonal angles: rho_pent = {rho_pent:.9f} "
      f"= 4 - (8/5)sqrt5 = {closed:.9f}  (T = 3/5 rational)")
# membership: in Z+Z*alpha? solve rho = m + n*alpha, m,n int?
sol_n = (rho_pent - round(rho_pent - 0)*0)       # brute scan
member = False
for m in range(-20, 21):
    n = (rho_pent - m)/ALPHA
    if abs(n - round(n)) < 1e-9:
        member = True; print(f"    rho_pent = {m} + {round(n)}*alpha")
if not member:
    print(f"    rho_pent NOT in Z + Z*alpha (needs the 1/5-index module: "
          f"(1/5)Z[sqrt5] -- Penrose-type 2D frequency module, TO VERIFY)")
rows["S_pent"] = S_pent; rows["rho_pent"] = rho_pent
rows["rho_pent_in_Z_Zalpha"] = member
json.dump(rows, open("swap_step1_results.json","w"), indent=1)
print("saved -> swap_step1_results.json")
