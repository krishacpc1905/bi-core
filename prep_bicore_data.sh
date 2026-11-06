#!/usr/bin/env bash
# Usage: ./prep_bicore_data.sh <konect_out_file> <output_dir> [num_update_edges=1000] [seed=42]
set -euo pipefail
in="${1:?konect out.* file}"; out="${2:?output dir}"; n="${3:-1000}"; seed="${4:-42}"
mkdir -p "$out"
grep -v '^[%#]' "$in" | awk 'NF>=2 {print $1, $2}' | sort -u > "$out/full.txt"
shuf -n "$n" --random-source=<(yes "$seed") "$out/full.txt" > "$out/sample.txt"
awk 'NR==FNR {s[$0]; next} !($0 in s)' "$out/sample.txt" "$out/full.txt" > "$out/base.txt"
wc -l "$out"/full.txt "$out"/sample.txt "$out"/base.txt
