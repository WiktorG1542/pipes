#!/usr/bin/env python3
import pathlib, sys, pandas as pd, itertools

csv = pathlib.Path("pipes.csv")
df  = pd.read_csv(csv, dtype=str, engine="python")

expected_cols = ["unsolved", "solved", "shape"]
assert list(df.columns) == expected_cols, f"header is {df.columns}, wanted {expected_cols}"

bad = []

for idx, row in df.iterrows():
    u, s, sh = row["unsolved"], row["solved"], row["shape"]

    if sh != "10x10":            bad.append((idx, "shape"))
    if len(u) != 200:            bad.append((idx, "unsolved"))
    if len(s.split(",")) != 100: bad.append((idx, "solved"))

if bad:
    print("WRONG, problems in rows:", bad[:10], "…")
    sys.exit(1)

print(f"OK, {len(df):,} rows look good")
