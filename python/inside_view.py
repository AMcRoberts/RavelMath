"""
inside_view.py -- what the substrate looks like FROM THE INSIDE.

Three demonstrations aimed at the philosophy, not the physics:

(1) SELF-CONCEALMENT / forced epistemic openness. An embedded observer
    with locality radius R knows theta only up to the level-R
    three-distance cell (~1/R resolution; Sturmian complexity: only
    R+2 distinct radius-R worlds exist). The setting/selection layer
    lives at stride d2. Measure I(radius-R observation ; setting) as a
    function of R. Prediction: ~0 bits until R ~ d2, then rising to
    the full 2 bits. Below the crossover, measurement dependence is
    IN PRINCIPLE invisible: the model is internally indistinguishable
    from one with free settings.

(2) ONE-HISTORY vs ALL-COPIES: unique ergodicity in action. For any
    local observable: (i) spatial average along ONE chain ("one person
    walking one universe"), (ii) exact window measure over the hull
    ("all the copies at once, weighted"), (iii) average along a
    DIFFERENT hull point (another phase offset). Strict ergodicity
    says all three are the same number for every internally-askable
    question.

(3) "COLLAPSE" = self-location refinement. Observing your radius-R
    neighborhood narrows your window interval; the conditional
    prediction for a distant letter computed from WINDOW GEOMETRY
    (all-copies picture) must equal the empirical frequency among
    sites sharing your observation (one-history picture).
"""
import math, json
import numpy as np

ALPHA = (math.sqrt(5)-1)/2
N = 1 << 21
theta = (np.arange(N, dtype=np.int64)*ALPHA) % 1.0
rows = {}

# ---------- (1) MI(R) between local observation and setting ----------
d2 = 1597
Q = np.floor(4*((d2*theta) % 1.0)).astype(np.int64)
def cell_ids(R):
    pts = np.sort(np.unique(np.array([(-j*ALPHA) % 1.0 for j in range(-1, R+1)])))
    return np.searchsorted(pts, theta, side="right") - 1, len(pts)
print("(1) embedded-observer information about the setting layer (d2=1597):")
print(f"{'R':>6} {'#worlds':>8} {'I(obs;setting) bits':>20}")
for R in (4, 16, 64, 256, 1024, 4096, 16384):
    cid, ncells = cell_ids(R)
    joint = np.zeros((ncells, 4))
    np.add.at(joint, (cid, Q), 1.0)
    joint /= joint.sum()
    pc = joint.sum(1, keepdims=True); pq = joint.sum(0, keepdims=True)
    nz = joint > 0
    I = float((joint[nz]*np.log2(joint[nz]/(pc@pq)[nz])).sum())
    bias = (np.count_nonzero(nz))/(2*N*math.log(2))
    print(f"{R:>6} {ncells:>8} {I:>20.5f}   (plug-in bias ~ {bias:.5f})")
    rows.setdefault("MI_R", []).append({"R": R, "I": I})

# ---------- (2) unique ergodicity: three computations, one number ----------
# observable: indicator that the radius-3 word around a site is a fixed word
R = 3
pts = np.sort(np.unique(np.array([(-j*ALPHA) % 1.0 for j in range(-1, R+1)])))
cid = np.searchsorted(pts, theta, side="right") - 1
target = 2                                     # an arbitrary radius-3 world
f_chain = float(np.mean(cid == target))        # (i) one history
lo, hi = pts[target], pts[target+1] if target+1 < len(pts) else 1.0
f_window = hi - lo                             # (ii) all copies, exact measure
phase = 0.7137113                              # (iii) a different hull point
th2 = (phase + np.arange(N, dtype=np.int64)*ALPHA) % 1.0
cid2 = np.searchsorted(pts, th2, side="right") - 1
f_other = float(np.mean(cid2 == target))
print(f"\n(2) unique ergodicity: one-history avg = {f_chain:.6f}, "
      f"window measure = {f_window:.6f}, other-universe avg = {f_other:.6f}")
rows["ergodicity"] = {"chain": f_chain, "window": f_window, "other": f_other}

# ---------- (3) collapse = self-location refinement ----------
# after observing your radius-3 cell, predict the letter at distance L
L = 55
letter = (theta < ALPHA)                       # letter 'a' iff theta in [0, alpha)
mask = (cid == target)
emp = float(np.mean(letter[(np.arange(N) + L) % N][mask]))     # one-history
# all-copies: measure of {t in [lo,hi): {t + L*alpha} < alpha} / (hi-lo)
delta = (L*ALPHA) % 1.0
grid = lo + (hi - lo)*(np.arange(200000) + 0.5)/200000
pred = float(np.mean(((grid + delta) % 1.0) < ALPHA))
print(f"(3) collapse-as-conditioning: empirical P(letter@+{L} | obs) = {emp:.6f}, "
      f"window-geometry prediction = {pred:.6f}")
rows["collapse"] = {"empirical": emp, "window_pred": pred}
json.dump(rows, open("inside_view_results.json","w"), indent=1)
print("saved -> inside_view_results.json")
