#!/bin/bash
# Run contact_boundary_4x4 on the 11 clean non-unimodular Pisot
# candidates.  contact_boundary_4x4 now does the contact-boundary
# pipeline AND the exact Q(β) eigenvalue in one invocation when
# given a matrix-path argument (see scripts/contact_boundary_4x4.cpp
# line 113-154: the dump_gb_matrix -> qbeta_eigenvalue pipeline
# is in-process).  So this script is a simple loop over candidates;
# the previous "dump then invoke qbeta_eigenvalue CLI" pattern is
# subsumed by the in-process path.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

OUT=out/cb_4x4_qbeta_results.txt
> "$OUT"

# Candidate matrices from the survey analysis.
declare -A CANDIDATES=(
    [1]="3 2 0 2 1 0 0 3 0 0 0 1 2 1 1 2"
    [12]="3 3 1 1 1 1 0 1 2 2 0 0 3 2 1 2"
    [13]="2 2 2 3 1 0 0 0 1 1 0 1 2 2 2 2"
    [21]="1 2 2 1 2 3 3 1 2 1 1 1 0 2 1 0"
    [38]="2 1 1 1 0 0 0 2 3 2 3 3 1 1 1 1"
    [45]="1 1 1 1 3 2 1 2 2 2 2 3 1 2 1 1"
    [53]="0 0 1 1 2 2 3 3 1 2 2 2 0 2 2 3"
    [70]="1 0 1 1 2 3 3 1 0 1 0 1 1 1 1 0"
    [71]="1 1 2 0 1 0 1 0 0 1 3 2 0 1 0 0"
    [75]="3 2 0 1 3 1 0 1 1 0 0 0 1 0 2 1"
    [82]="0 1 1 2 0 3 2 1 1 3 2 1 0 1 1 0"
)

i=0
for idx in "${!CANDIDATES[@]}"; do
    i=$((i+1))
    matrix="${CANDIDATES[$idx]}"
    name="rnd${idx}_nonunimod"
    matrix_path="/tmp/cb_matrix_${idx}.txt"

    # Try to read beta from a candidates.txt if it exists (optional);
    # otherwise let contact_boundary_4x4 compute it from the matrix.
    if [ -f /tmp/candidates.txt ]; then
        beta=$(grep "^$idx " /tmp/candidates.txt | awk '{print $2}')
        args="$matrix $beta $name $matrix_path"
    else
        args="$matrix $name $matrix_path"
    fi

    echo "=== [$i/11] $name ===" | tee -a "$OUT"
    timeout 120 ./out/contact_boundary_4x4 $args 2>&1 | tee -a "$OUT"
    echo "" | tee -a "$OUT"
done

echo "" | tee -a "$OUT"
echo "=== Summary ===" | tee -a "$OUT"
python3 - << EOF | tee -a "$OUT"
import re
with open('$OUT') as f:
    text = f.read()
records = []
cur = {}
for line in text.split('\n'):
    m = re.search(r'=== \[(\d+)/11\] (rnd\d+_nonunimod) ===', line)
    if m:
        if cur.get('name'):
            records.append(cur)
        cur = {'i': int(m.group(1)), 'name': m.group(2)}
    m = re.search(r'beta=([\d.]+)', line)
    if m and cur and 'beta' not in cur:
        cur['beta'] = float(m.group(1))
    m = re.search(r'lambda\(G_B\)=([\d.]+)', line)
    if m and cur:
        cur['lambda_dp'] = float(m.group(1))
    m = re.search(r'qbeta\(G_B\): lambda=([\d.]+)', line)
    if m and cur:
        cur['lambda_qb'] = float(m.group(1))
if cur.get('name'): records.append(cur)

print(f"{'name':<22} {'β':>8} {'λ (dp)':>10} {'λ (Q(β))':>15} {'ratio':>10}")
for r in records:
    if 'lambda_dp' in r and 'lambda_qb' in r:
        ratio = r['lambda_qb'] / r['lambda_dp']
        print(f"  {r['name']:<22} {r['beta']:>8.4f} {r['lambda_dp']:>10.4f} {r['lambda_qb']:>15.10f} {ratio:>10.6f}")
EOF
