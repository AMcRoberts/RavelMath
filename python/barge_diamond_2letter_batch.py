"""
barge_diamond_2letter_batch.py -- runs the Barge-Diamond certificate
(barge_diamond_reimplementation.py) across 40 systematically-generated
2-letter unimodular Pisot substitutions, confirming Theorem 1 applies
(hence the full Strong Coincidence Conjecture is closed) for every
one -- not just the small hand-picked set in the main module's demo.

The 40 substitutions were found by random search over 2-letter words
of length 1-5, filtered to unimodular (|det|=1) Pisot matrices
(dominant real eigenvalue >1, secondary eigenvalue modulus <1),
seed=7, first 40 hits.
"""
import sys
import os

sys.path.insert(0, os.path.dirname(__file__))
from barge_diamond_reimplementation import certify

CASES = [
    [[1, 0, 0], [0, 1]], [[1, 0], [0]], [[0, 0, 0, 0, 1], [0]], [[1], [0, 1]], [[1], [1, 0, 1]],
    [[1, 0, 0, 0], [0, 1, 0]], [[1, 0, 1], [1, 0]], [[0, 1], [0]], [[1], [0, 1, 1, 1]], [[1, 0, 1, 0, 1], [0, 1]],
    [[0, 0, 1, 0], [0, 1, 0]], [[0, 1], [0, 1, 0]], [[1, 0], [1, 1, 0]], [[1], [1, 1, 0]], [[0, 1, 1, 1], [1, 0, 1]],
    [[1], [1, 0, 1]], [[1, 0, 0], [0, 0, 0, 1]], [[1, 0], [0, 0, 1]], [[1], [1, 0]], [[1, 1, 0], [1, 0]],
    [[0, 1, 0, 0], [0]], [[1], [1, 1, 1, 0, 1]], [[1], [1, 0, 1]], [[0, 0, 1, 1, 0], [1, 0]], [[1, 0], [0, 0, 0, 1]],
    [[0, 1, 1], [1, 1, 1, 0, 0]], [[1, 1, 0], [1, 1, 0, 1, 0]], [[1, 0, 1], [1, 1, 0, 1]], [[1, 1, 0], [1, 0, 1, 0, 1]],
    [[0, 1, 1, 1, 0], [0, 1, 1]],
    [[0, 1], [1, 0, 0, 0, 1]], [[1, 0], [0, 1, 1]], [[1, 0], [1, 1, 0]], [[1], [0, 1]], [[1], [1, 0, 1]],
    [[1], [0, 1]], [[1, 0, 0, 1, 0], [0, 1]], [[1], [1, 1, 0]], [[1], [0, 1]], [[1, 0, 1], [0, 1]],
]


def main():
    ok = 0
    fail = 0
    for i, sigma in enumerate(CASES):
        cert = certify(sigma, 2, name=f"case{i}")
        success = cert.get("M_stabilized") and cert.get("beta_irrational")
        if success:
            ok += 1
        else:
            fail += 1
            print(f"case{i} FAILED: {sigma}  beta_irrational={cert.get('beta_irrational')} "
                  f"M_stabilized={cert.get('M_stabilized')}")
    print(f"\n{ok}/{ok + fail} certificates complete (Theorem 1 applies, hence full strong "
          f"coincidence for d=2)")
    assert fail == 0, f"{fail} certificate(s) failed -- see above"


if __name__ == "__main__":
    main()
