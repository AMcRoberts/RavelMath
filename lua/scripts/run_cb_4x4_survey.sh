#!/bin/bash
# Run contact_boundary_4x4 on all 87 non-unimodular Pisot candidates.
# Each candidate gets a 30-second timeout; results are written to out/cb_4x4_results.txt.

set -e
cd "$(dirname "$0")/.."

OUT=out/cb_4x4_results.txt
> "$OUT"

i=0
while read -r idx beta det m00 m01 m02 m03 m10 m11 m12 m13 m20 m21 m22 m23 m30 m31 m32 m33; do
    i=$((i+1))
    name="rnd${idx}_nonunimod"
    # 30-second timeout per candidate
    timeout 30 ./out/contact_boundary_4x4 \
        $m00 $m01 $m02 $m03 \
        $m10 $m11 $m12 $m13 \
        $m20 $m21 $m22 $m23 \
        $m30 $m31 $m32 $m33 \
        "$beta" "$name" 2>&1 | tee -a "$OUT"
    echo "---" | tee -a "$OUT"
done < /tmp/candidates.txt

echo ""
echo "DONE: results in $OUT"
echo "Total candidates: $i"
